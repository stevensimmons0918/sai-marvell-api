/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

/*ADDRESSES*/

#define MG0_BASE_ADDRESS 0x7f900000
#define DFX_BASE 0x7f980000
#define AC5_MMAP_ADDR DFX_BASE + 0x4000
#define AC5X_MMAP_ADDR DFX_BASE + 0x4000
#define AC5_EFUSE_CONTROL_REG (AC5_MMAP_ADDR + 0x8)
#define AC5X_EFUSE_CONTROL_REG  (AC5X_MMAP_ADDR + 0x8)

#define ID_BASE_ADDRESS_OFFSET_FROM_CONTROL 0xEF8
#define HD_BASE_ADDRESS_OFFSET_FROM_CONTROL 0x1108
#define MAX_OFFSET_FROM_CONTROL 0x12F8

#define ID_STATUS_REGION_0 0
#define ID_STATUS_REGION_1 1
#define ID_STATUS_REGION_2 2
#define INDEX_COUNT_DOWN 0
#define INDEX_COUNT_UP 1
#define APPLY_DUPLICATION 1
#define REMOVE_DUPLICATION 0
#define IS_KEY_TYPE 1
#define IS_NOT_KEY_TYPE 0
#define FD 0
#define PTR 1
#define CREATE 1
#define DONT_CREATE 0
#define MEMORY_FLAG 0xCAFEBABE
#define GEN_KEY_TYPE_AES 3
#define BOOT_MODE_MASK_BITS_UNION 0x3FF0000

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef LINUX
	#include <string.h>
	#include <stdint.h>
	#include <sys/mman.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include "fuselist.h"
#else /*UBoot*/
	#include <linux/string.h>
	#include <linux/sizes.h>
	#include <mvebu/efuse_tool.h>
	#include <linux/delay.h>
	#include <common.h>
#endif

bool simulation = 0, is_ac5, verbose = 0;
void *fuse_control_reg_ptr = NULL;
mv_u32 print_mem_ptr = 0;
enum addressee {ORIGIN,
				ALTERED,
				FUSE_REGISTERS};


#ifdef LINUX
	int originfd = -1, alteredfd = -1, simulationfd = -1;

	void *org_destination = &originfd;
	void *alt_destination = &alteredfd;
	void *sim_destination = &simulationfd;
	#define ORIGIN_FILE_NAME "/tmp/fuse_info.bin"
	#define ALTERED_FILE_NAME "/tmp/altered_fuse_info.bin"
	#define SIM_SRC_FILE_NAME "/tmp/simulation_init.bin"
#else
	#define ORIGIN_MEM_FLAG 0x7f980000
	#define ORIGIN_MEM_START (ORIGIN_MEM_FLAG + 4)
	#define ALTERED_MEM_FLAG (ORIGIN_MEM_FLAG + 0x1300)
	#define ALTERED_MEM_START (ORIGIN_MEM_FLAG + 0x1304)
	#define SIMULATION_MEM_FLAG (ORIGIN_MEM_FLAG + 0x2600)
	#define SIMULATION_MEM_START (ORIGIN_MEM_FLAG + 0x2604)
	#define SIMULATION_FLAG_ADDRESS (ORIGIN_MEM_FLAG + 0x3904)

	void *org_destination = (void *)ORIGIN_MEM_START;
	void *alt_destination = (void *)ALTERED_MEM_START;
	void *sim_destination = (void *)SIMULATION_MEM_START;
#endif

struct SecurityIDFuse_info securityIDFuse_sysmap[] = {
/*Name                                         writeAddr                                      bitMask     idFuseRange            readBitMask*/
{ "secure_mode",                               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x3,        ID_STATUS_REGION_0,    0x1},
{ "jtag_permanent_disable",                    ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC,        ID_STATUS_REGION_0,    0x2},
{ "jtag_soft_disable",                         ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x30,       ID_STATUS_REGION_0,    0x4},
{ "uart_soft_disable",                         ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC0,       ID_STATUS_REGION_0,    0x8},
{ "uart_permanent_disable",                    ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x300,      ID_STATUS_REGION_0,    0x10},
{ "ssmi_soft_disable",                         ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC00,      ID_STATUS_REGION_0,    0x20},
{ "ssmi_permanent_disable",                    ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x3000,     ID_STATUS_REGION_0,    0x40},
{ "i2c_slave_soft_disable",                    ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC000,     ID_STATUS_REGION_0,    0x80},
{ "i2c_slave_permanent_disable",               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x30000,    ID_STATUS_REGION_0,    0x100},
{ "encrypt_boot",                              ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC0000,    ID_STATUS_REGION_0,    0x200},
{ "boot_mode_bit_0_nand",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x300000,   ID_STATUS_REGION_0,    0x400},
{ "boot_mode_bit_1_spi_nor",                   ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC00000,   ID_STATUS_REGION_0,    0x800},
{ "boot_mode_bit_2_spi_nand",                  ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x3000000,  ID_STATUS_REGION_0,    0x1000},
{ "boot_mode_bit_3_emmc",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC000000,  ID_STATUS_REGION_0,    0x2000},
{ "boot_mode_bit_4_xmodem",                    ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0x30000000, ID_STATUS_REGION_0,    0x4000},
{ "perform_entropy_self_test",                 ID_BASE_ADDRESS_OFFSET_FROM_CONTROL,           0xC0000000, ID_STATUS_REGION_0,    0x8000},

{ "oob_port_secure_access_enable",             ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0x3,        ID_STATUS_REGION_0,    0x10000},
{ "pcie_secure_access_enable",                 ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xC,        ID_STATUS_REGION_0,    0x20000},
/*TODO: Check if this field is the Runit secure.. bit 18 ,read region 0 statur, cider.*/
{ "i2c/nfc/uart_secure_access_enable",         ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0x30,       ID_STATUS_REGION_0,    0x40000},
{ "gpios_secure_access_enable",                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xC0,       ID_STATUS_REGION_0,    0x80000},
{ "dma0_secure_access_enable",                 ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0x300,      ID_STATUS_REGION_0,    0x100000},
{ "dma1_secure_access_enable",                 ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xC00,      ID_STATUS_REGION_0,    0x200000},
{ "key_0_type",                                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xF000,     ID_STATUS_REGION_0,    0xC00000},
{ "key_1_type",                                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xF0000,    ID_STATUS_REGION_0,    0x3000000},
{ "key_2_type",                                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xF00000,   ID_STATUS_REGION_0,    0xC000000},
{ "key_3_type",                                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xF000000,  ID_STATUS_REGION_0,    0x30000000},
/*{ "Reserved",                                  ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0x30000000, ID_STATUS_REGION_0,  31, 0x40000000},*/
{ "id_fuse_range_0_lock",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x4,     0xC0000000, ID_STATUS_REGION_0,    0x80000000},

{ "chip_manufacturing_cm",                     ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x3,        ID_STATUS_REGION_1,    0x1},
{ "device_mfg_dm",                             ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC,        ID_STATUS_REGION_1,    0x2},
{ "device_deplotment_dd",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x30,       ID_STATUS_REGION_1,    0x4},
{ "failure_analysis_fa",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC0,       ID_STATUS_REGION_1,    0x8},
{ "revoke_key_id_bit_0",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x300,      ID_STATUS_REGION_1,    0x10},
{ "revoke_key_id_bit_1",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC00,      ID_STATUS_REGION_1,    0x20},
{ "revoke_key_id_bit_2",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x3000,     ID_STATUS_REGION_1,    0x40},
{ "revoke_key_id_bit_3",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC000,     ID_STATUS_REGION_1,    0x80},
{ "revoke_key_id_bit_4",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x30000,    ID_STATUS_REGION_1,    0x100},
{ "revoke_key_id_bit_5",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC0000,    ID_STATUS_REGION_1,    0x200},
{ "revoke_key_id_bit_6",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x300000,   ID_STATUS_REGION_1,    0x400},
{ "revoke_key_id_bit_7",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC00000,   ID_STATUS_REGION_1,    0x800},
{ "revoke_key_id_bit_8",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x3000000,  ID_STATUS_REGION_1,    0x1000},
{ "revoke_key_id_bit_9",                       ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC000000,  ID_STATUS_REGION_1,    0x2000},
{ "revoke_key_id_bit_10",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0x30000000, ID_STATUS_REGION_1,    0x4000},
{ "revoke_key_id_bit_11",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x8,     0xC0000000, ID_STATUS_REGION_1,    0x8000},

{ "revoke_key_id_bit_12",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0x3,        ID_STATUS_REGION_1,    0x10000},
{ "revoke_key_id_bit_13",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0xC,        ID_STATUS_REGION_1,    0x20000},
{ "revoke_key_id_bit_14",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0x30,       ID_STATUS_REGION_1,    0x40000},
{ "revoke_key_id_bit_15",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0xC0,       ID_STATUS_REGION_1,    0x80000},
{ "key_0_valid",                               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0x300,      ID_STATUS_REGION_1,    0x100000},
{ "key_1_valid",                               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0xC00,      ID_STATUS_REGION_1,    0x200000},
{ "key_2_valid",                               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0x3000,     ID_STATUS_REGION_1,    0x400000},
{ "key_3_valid",                               ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0xC000,     ID_STATUS_REGION_1,    0x800000},
{ "bootrom_message_disable",                   ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0X30000,    ID_STATUS_REGION_1,    0x1000000},
/*{ "Reserved",                                  ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0x3F000000, ID_STATUS_REGION_1,  31, 0x7E000000},*/
{ "id_fuse_range_1_lock",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xC,     0xC0000000, ID_STATUS_REGION_1,    0x80000000},
{ "security_disable",                          ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x18,    0x3,        ID_STATUS_REGION_2,    0},
{ "secure_rom",                                ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x18,    0xC,        ID_STATUS_REGION_2,    0},
{ "id_fuse_range_2_lock",                      ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x18,    0xC0000000, ID_STATUS_REGION_2,    0},
{ "last",                                      0,                                            0,            0,                    0}
};



struct SecurityHDFuse_info HDsecurityFuse_sysmap[] = {
/*Name               logical Bit Size        Starting adress                 */
{ "jtaguuid",        64,           HD_BASE_ADDRESS_OFFSET_FROM_CONTROL,      },
{ "token",           64,           HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x20},
{ "productid",       32,           HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x40},
{ "oemkeyhash",      256,          HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x50},
{ "generickey0",     256,          HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xA0},
{ "generickey1",     256,          HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0xF0},
{ "generickey2",     256,          HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x140},
{ "generickey3",     256,          HD_BASE_ADDRESS_OFFSET_FROM_CONTROL + 0x190},
{ "last",            0,            0,                                         }
};


char const *help_menu[] = {
	"secure_mode                        Enables the Secured Mode\n" ,
	"\nWhen secure mode is enabled, the following fuses will take action as follows:\n",
	"____________________________________________________________________________\n\n",
	"jtag_permanent_disable             JTAG is permanently locked by hardware and cannot unlock\n",
	"jtag_soft_disable                  JTAG is soft lock by hardware, but can be unlocked through ROM/FW\n",/*1-JTAG_SOFT_DISABLE*/
	"uart_soft_disable                  UART can be soft lock through ROM / Secured FW\n",/*3-UART_SOFT_DISABLE*/
	"ssmi_soft_disable                  SSMI can be soft lock through ROM / Secured FW\n",/*5-SSMI_SOFT_DISABLE*/
	"ssmi_permanent_disable             SSMI port is permanently locked by ROM\n",/*6-SSMI_PERMANENT_DISABLE*/
	"i2c_slave_soft_disable             I2C can be soft lock through ROM / Secured FW\n",/*7-I2C_SLAVE_SOFT_DISABLE*/
	"i2c_slave_permanent_disable        I2C port is permanently locked by ROM\n",/*8-I2C_SLAVE_PERMANENT_DISABLE*/
	"\nThe follwing fields enables SAR controlled boot mode:\n",
	"  boot_mode_bit_0_nand\n",/*10-BOOT_Mode_bit0*/
	"  boot_mode_bit_1_spi_nor\n",
	"  boot_mode_bit_2_spi_nand\n",
	"  boot_mode_bit_3_emmc\n",
	"  boot_mode_bit_4_xmodem\n",

	"\nThe ROM decodes those fuse fields bits to require secure access to (registers):\n",
	"  oob_port_secure_access_enable\n",/*16-OOB_port_secure_access_enable*/
	"  pcie_secure_access_enable\n",/*17-PCIe_secure_access_enable*/
	"  i2c/nfc/uart_secure_access_enable\n",/*18-I2C_NFC_UART_secure_access_enable*/
	"  gpios_secure_access_enable\n",/*19-GPIOs_secure_access_enable*/
	"  dma0_secure_access_enable\n",/*20-DMA0_secure_access_enable*/
	"  dma1_secure_access_enable\n",/*21-DMA1_secure_access_enable*/

	"\nThe ROM decodes those fuse bits for key revocation of the manifests\n",/*28-Revoke_Key_ID_bit_0*/
	"  revoke_key_id_bit_0\n",
	"  revoke_key_id_bit_1\n",
	"  revoke_key_id_bit_2\n",
	"  revoke_key_id_bit_3\n",
	"  revoke_key_id_bit_4\n",
	"  revoke_key_id_bit_5\n",
	"  revoke_key_id_bit_6\n",
	"  revoke_key_id_bit_7\n",
	"  revoke_key_id_bit_8\n",
	"  revoke_key_id_bit_9\n",
	"  revoke_key_id_bit_10\n",
	"  revoke_key_id_bit_11\n",
	"  revoke_key_id_bit_12\n",
	"  revoke_key_id_bit_13\n",
	"  revoke_key_id_bit_14\n",
	"  revoke_key_id_bit_15\n",

	"\nbootrom_message_disable          The ROM decodes this bit and once set disables BootROM messages.\n"/*30-BootROM_Message_Disable*/
	"                                     Note - Exceptions are:\n"
	"                                       1. Escape sequence recognized\n"
	"                                       2. No image found on flash, and uart port is enabled\n",
	"__________________________________________________________________________________________\n\n",

	"perform_entropy_self_test           When set, the ROM will perform entropy self-test in secure mode.\n",/*15-Perform_Entropy_self_test*/
	"encrypt_boot                        Enabled images to be encrypted\n",/*9-ENCRYPT_BOOT*/
	"key_x_type                          If '1' - the key is valid. For key type of OEM HASH and AES\n",
	"                                    the highest valid key is used by the bootRom\n",/*22-Key_0_Type*/
	"                                    0 - Unused   1 - GENERIC   2 - OEM KEY HASH   3 - AES KEY\n",
	"                                    x can be any one of the values 0 - 3\n",
	"id_fuse_range_0_lock                Locks range 0 of the eFuse block for further burning\n",/*23-ID_Fuse_range_0_lock*/
	"id_fuse_range_1_lock                Locks range 1 of the eFuse block for further burning\n",/*32-ID_Fuse_range_1_lock*/
	"chip_manufacturing_cm               Life chip state - Chip Manufacture\n",/*24-CHIP_MANUFACTURING_CM*/
	"device_mfg_dm                       Life chip state - Deive Manufacture\n",/*25-DEVICE_MFG_DM*/
	"device_deplotment_dd                Life chip state - Device deployment\n",/*26-DEVICE_DEPLOYMENT_DD*/
	"failure_analysis_fa                 Life chip state - Failure Analysis, soft JTAG/UART port is unlock.\n",/*27-FAILURE_ANALYSIS_FA*/
	"                                    If Secure Mode efuse bit = 1 and JTAG permanent efuse bit = 0,\n",
	"                                    then the chip is in non-secure mode.\n",
	"                                    Generic keys are locked unless they are of type 'OEM HASH' \n",
	"key_x_valid                         '1' -  the key is valid. For key type of OEM HASH and AES,\n"
	"                                    the highest valid key is used by the bootRom\n",/*29-Key_0_Valid*/
	"                                    x can be any one of the values 0 - 3\n",
	"security_disable                    The ROM decodes this bit and if set, bypass secured boot and disable ARM A55 Crypto extentions\n",
	"secure_rom                          The ROM decodes this bit for disabling the read back of secured portion of the ROM, securing\n"
	"                                    CCSS regfile and disabling UART based ROM 'internal' debug logging function\n"
	"                                    (doesn't affect 'external' ROM messages)\n",
	NULL};

char const *help_info_hd[] = {
	"jtaguuid                            If the TIM contains a JTAG control block, the ROM will compare it with this fuse to\n",
	"                                    unlock the JTAG port\n", /*33 - JTAG_UUID*/
	"token                               If the TIM contains a Token, the ROM will compare it with this fuse.\n", /*34 - TOKEN*/
	"productid                           The ROM compares with the TIM's UniqueID field for SW image per product type\n", /*35 - PRODUCT_ID*/
	"oemkeyhash                          The OEM key is used by the ROM to compare during authentication.\n", /*36 - OEM KEY HASH*/
	"generickeyx                         These are Generic Key fileds used by the ROM for various purposes as configured by \n",
	"                                    the Key Type fields in the Security ID eFuse.\n",/*37 -GENERIC KEY*/
	"                                    x can be any one of the values 0 - 3\n",
	NULL};

static const mv_u8 hd_fuse_en[4] = {0x30, 0x28, 0xf9, 0xaf}, id_write_init_vals[4] = {0x70, 0x28, 0xf9, 0xaf}, id_read_init_vals[4] = {0x70, 0x28, 0xf9,0x2f};
static const mv_u32 maskarr[3] ={0xff, 0xffff, 0xffffff}, byte_mask_arr[] = {0xff, 0xff00, 0xff0000, 0xff000000};


void mvebu_efuse_error_print(int arg_count, ...) {
	va_list arglist;
	int function_indetifier;
	int number;
	char *name, *name2;

	va_start(arglist, arg_count);
	function_indetifier = va_arg(arglist, int);

	switch (function_indetifier){

		case MVEBU_EFUSE_WRITE_FUSE_PROCEDURE:
		case MVEBU_EFUSE_READ_FUSE_PROCEDURE:
		case INPUT_VALUE_SAFETY_CHECK:
			if (function_indetifier == MVEBU_EFUSE_READ_FUSE_PROCEDURE)
				name = "read_fuse_procedure";
			else if (function_indetifier == MVEBU_EFUSE_WRITE_FUSE_PROCEDURE)
				name = "write_fuse_procedure";
			else
				name = "input_value_safety_check";

			name2 = va_arg(arglist, char *);
			printf("%s(%s) failed.\n", name, name2);
			break;

		case MVEBU_EFUSE_SETUP_DB:
		case MVEBU_EFUSE_ADDRESSEE_SWITCH_CASE:
		case MVEBU_EFUSE_INITIALIZE_RESOURCE:

			if (function_indetifier == MVEBU_EFUSE_SETUP_DB)
				name = "setup_DB";
			else
				name = "addressee_switch_case";

			number = va_arg(arglist, int);
			switch (number) {
			case ORIGIN:
				name2 = "ORIGIN";
				break;
			case ALTERED:
				name2 = "ALTERED";
				break;
			case FUSE_REGISTERS:
				name2 = "FUSE_REGISTERS";
				break;
			default:
				name2 = "no such addressee!\n";
				break;
			}
			printf("%s(enum addressee: %s) failed.\n", name, name2);
			break;

		/*case MVEBU_EFUSE_FILE_WRITE:	 */
		case MVEBU_EFUSE_VALUE_WRITE:
		case MVEBU_EFUSE_FUSE_WRITE:
		case  MVEBU_EFUSE_APPLY_CHANGES:
		case MVEBU_EFUSE_READ_ALL:
		case MVEBU_EFUSE_FUSE_READ_CMD:
		case MVEBU_EFUSE_INIT_DB:
		case MVEBU_EFUSE_VALUE_READ:

		#ifdef LINUX
		case MVEBU_EFUSE_FILE_WRITE:
		case MVEBU_EFUSE_FILE_READ:
			if (function_indetifier == MVEBU_EFUSE_FILE_WRITE)
				name = "file_write";
			else if (function_indetifier == MVEBU_EFUSE_FILE_READ)
				name = "file_read";
		#endif
			if (function_indetifier == MVEBU_EFUSE_VALUE_READ)
				name = "value_read";
			else if (function_indetifier == MVEBU_EFUSE_VALUE_WRITE)
				name = "value_write";
			else if (function_indetifier == MVEBU_EFUSE_FUSE_WRITE)
				name = "mvebu_efuse_fuse_write";
			else if (function_indetifier ==  MVEBU_EFUSE_APPLY_CHANGES)
				name = "apply";
			else if (function_indetifier == MVEBU_EFUSE_READ_ALL)
				name = "mvebu_efuse_read_all";
			else if (function_indetifier == MVEBU_EFUSE_FUSE_READ_CMD)
				name = "mvebu_efuse_fuse_read_cmd";
			else if (function_indetifier == MVEBU_EFUSE_DETECT_DEVICE)
				name = "detect_device";
			else
				name = "mvebu_efuse_init_DB";

			printf("%s failed.\n", name);
			break;


		case MVEBU_EFUSE_ID_BIT_DUPLICATION:
		case MVEBU_EFUSE_FIND_BITMASK_LOCATION:
			if (function_indetifier == MVEBU_EFUSE_ID_BIT_DUPLICATION)
				name = "id_bit_duplication";
			else
				name = "find_bitmask_location";

			number = va_arg(arglist, int);/*fuse index*/
			printf("%s(%s) failed.\n", name, securityIDFuse_sysmap[number].name);
			break;
		#ifdef LINUX
		case MVEBU_EFUSE_SAFE_OPEN_FD:
			name = va_arg(arglist, char *);
			number = va_arg(arglist, int);
			printf("%s (path: %s,  create: %d) failed.\n", "safe_open_fd", name,number);
			break;
		#endif

		default:
			printf("Unresolved case. mvebu_efuse_error_print failed.\n");
			break;
	}

	va_end(arglist);
}

static char get_input(void){
	char ans;

	#ifdef LINUX
		ans = getc(stdin);
	#else /*Uboot*/
		ans = getc();
	#endif
	return ans;
}

#ifndef LINUX


static void fflush(int fd){

}
static void sleep(float seconds){
	int i, time = seconds * 1000;
	for (i = 0; i < time; i++) {
		mdelay(1);
	}
}



#endif

static int find_id_fuse_index(char *fusename){
	int i;

	if (!fusename)
		return -1;
	for (i = 0; (securityIDFuse_sysmap[i].writeAddr != 0); i++) {
		if (strcmp(fusename, securityIDFuse_sysmap[i].name) == 0)
			return i;
	}
	return -1;
}

static int find_hd_fuse_index(char *fusename){
	int i;
	if (!fusename)
		return -1;
	for (i = 0; (HDsecurityFuse_sysmap[i].startAddr != 0); i++) {
		if (strcmp(fusename, HDsecurityFuse_sysmap[i].name) == 0)
			return i;
	}
	return -1;
}


/**
* Function description:
*  detect_fuse finds fuse index in ID/HD fuse table and
*  indicates ID/HD aswell.
*
*
* @param  fuse_name - The fuse name to detect.
* @param  idOrHd - pointer to int variable stores 0/1/ according
*         to function result.
* @param  fuseindex - pointer to int variable that will store
*         fuseindex.
*/
static void detect_fuse(char *fuse_name, int *idOrHd, int *fuseindex){

	*fuseindex = find_hd_fuse_index(fuse_name);
	if (*fuseindex == (-1)) {
		*fuseindex = find_id_fuse_index(fuse_name);
		if (*fuseindex == (-1)){
			printf("Invalid fuse name: %s. \n", fuse_name);
			*idOrHd = -1;
			return;
		}
		else
			*idOrHd = 0; /* ID fuse found */
	}
	else
		*idOrHd = 1; /* HD fuse found */
}


#ifdef LINUX
/**This function writes bytes from given buffer to the file
 *  	  specified.
 *
 * @param fd - file-descriptor of the destination file.
 * @param buf - the buffer given from which the bytes will be
 *        written into the file.
 * @param numOfBytes - number of bytes to read from file.
 * @param offset - numeric offset value from given postion.
 *        SEEK_SET(= 0) - The file offset is set to offset bytes.
 *
 * @retval (-1) - if an error has occured and prints the error
 *  	 trigger.
 * @retval positive int - if function finished successfuly,
 *         return number of bytes written.
 */
static int file_write(int fd, mv_u8 *buf, int numOfBytes, off_t offset ){
	int i, errsv;

	if (fd < 0 || buf == NULL || numOfBytes > 0x4000 || offset > 0x4000){
		printf("Error in file_write\nfd = %d buf = %hhn numOfBytes = %d  offset = %lx.\n", fd, buf, numOfBytes, offset);
		return -1;
	}

	i = lseek(fd, offset, SEEK_SET);
	if (i == -1) {
		errsv = errno;
		printf("LSEEK ERROR ! \n errno = %d\n ", errsv);
		perror("Error Description: ");
		return -1;
	}
	i = write(fd, buf, numOfBytes);
	if (i == (-1)) {
		errsv = errno;
		printf("WRITE ERROR ! \n errno = %d\n ", errsv);
		perror("Error Description: ");
		return -1;
	}
	return i;
}
#endif

#ifdef LINUX
/** This function reads bytes from file into given mv_u8 buffer with option to offset.
 *
 * @param fd - sim file file-descriptor.
 * @param buf - the buffer given to store the read bytes from file.
 * @param numOfBytes - number of bytes to read from file.
 * @param offset - numeric offset value from given position.
 *       SEEK_SET(= 0) - The file offset is set to offset bytes.
 *
 * @retval (-1) - if an error has occurred and prints the error trigger.
 * @retval  positive int - if function finished successfully, return number of bytes read.
 */
static int file_read(int fd, mv_u8 *buf, int numOfBytes, off_t offset ){
	int i, errsv;
	if (fd < 0 || buf == NULL || numOfBytes > 0x4000 || offset > 0x4000){
		printf("Error in file_read\nfd = %d buf = %hhn numOfBytes = %d  offset = 0x%lx.\n", fd, buf, numOfBytes, offset);
		return -1;
	}


	i = lseek(fd, offset, SEEK_SET);
	if (i == -1) {
		errsv = errno;
		printf("LSEEK ERROR ! \n errno = %d\n ", errsv);
		perror("Error description:");
		return -1;
	}

	i = read(fd, buf, numOfBytes);
	if (i == (-1)) {
		errsv = errno;
		printf("READ ERROR ! \n errno = %d\n ", errsv);
		perror("Error description:");
		return -1;
	}
/*	printf("Number of bytes read is :%d \n",i);	  */
	if (i == 0) {
		printf("Bytes read is 0!\n");
		return -1;
	}
	return i;
}
#endif

/**This function recieve uint8 array sized 4 and returns uint32_t.
 *
 * @param bytes - given uint8_t array sized four to build one
 *  	 uint32_t value.
 * @param startIndex - initial index.
 * @param countup - if 1, increment index by 1 for each action.
 *  	 else decrese index value by 1 for each action.
 *
 */

static mv_u32 uint8arrTouint32(mv_u8 *bytes, int startIndex, bool countup){
	mv_u32 reg32b = 0;
	int i = 0;
	for (i = 0; i < 4; i++)
		if (countup == 0)
			reg32b |= bytes[startIndex--] << (i * 8);
		else
			reg32b |= bytes[startIndex++] << (i * 8);

	return reg32b;
}

/**
 * uint32Touint8arr recieve a 32 bit number and build a
 * numofbyte sized uint8_t array.
 *
 * @param bytes - Given uint8_t array to store inputNumber.
 *  			inputNumber LSB's will be stored in the lowest
 *  			index and so on..
 * @param numofbytes - Number of bytes out of the four to keep.
 * @param inputNumber - - The 32 bit number to devide.
 * @param startIndex - Initial index to start from in the given array.
 */
static void uint32Touint8arr(mv_u8 *bytes, int numofbytes, mv_u32 inputNumber, int startIndex){
	int i;

	for (i = 0; i < numofbytes; i++)
		bytes[startIndex + i] = ((inputNumber & byte_mask_arr[i]) >> i*8);
}

/**
 * strToByteArr checks basic input assumptions and then convert
 * str to mv_u8 array.
 *
 * @param str
 * @param bytes
 * @param sizeOfbytes
 *
 * @return int
 */
static int strToByteArr(char *str, mv_u8 *bytes, int sizeOfbytes ){
	int i, strlength = 0, res = 0, msb = 0, index = 0;

	memset(bytes, 0, sizeOfbytes);
	strlength = strlen(str);
	if (strlength > 66){ /* string is too long for any field (with 0x)*/
		printf("Entered value is too large for any field.\n");
		return -1;
	}
	if (strlength <= 2) { /*minus the hex prefix*/
		printf("Error: please enter a numeric value. \n");
		return -1;
	}
	if (str[0] != '0' || (str[1] != 'x') ){/*make sure that the number is with 0x prefix*/
		printf("No '0x' prefix detected.  \n");
		return -1;
	}

	for (i = strlength - 1; i > 1; i--) { /*assuming hex number of the form 0x..*/
		if (('0' <= str[i]) && (str[i] <= '9')) /*0-9*/
			res =  str[i] - '0';

		else if (('A' <= str[i]) && (str[i] <= 'F')) /*A-F*/
			res = str[i] - 'A' + 0xa;

		else if (('a' <= str[i]) && (str[i] <= 'f')) /*a-f*/
			res = str[i] - 'a' + 0xa;

		else {
			printf("Encountered with invalid character!\nInput must be hex based. character encountered : %c\n", str[i]);
			return -1;
		}
		if (msb == 1) {
			res = res << 4;
			bytes[index] += res;
			msb = 0;
			index++;
		}
		else {
			bytes[index] += res;
			msb = 1;
		}
	}
	return 1;
}

void mvebu_efuse_print_usage(void){
	printf("\nUsage:\n"
			"  efuse write 'fuse_name' <value>  - Write <value> to fuse_name corresponding fields. \n"
			"  efuse read 'fuse_name'           - Read the 'fuse_name' value.\n"
			"  efuse read                       - Read all available fuses values. \n"
			"  efuse status                     - Prints the modified fuses. \n"
			"  efuse apply                      - Prints the modified fuses and burn them if permission granted. \n"
			"  efuse apply verbose              - Prints the modified fuses and burn them if permission granted. Addresses and values are printed\n"
			"  efuse simulation <on/off>        - Simulate registers, no actual fuse register R/W is done.\n"
		#ifdef LINUX
		    "  efuse --help                     - Prints detailed information and all fuses names. \n"
		    "\nCompilation flags:\n"
			"  [SIM=y]   - Use gcc. if SIM not used then aarch64-linux-gnu-gcc is used.\n"
			"  [DEBUG]   - Enable writing to SRAM instead of fuse registers.\n  In addition prints additional data that may help to debug.\n"
			"  [LINUX=y] - Builds the tool for Linux enviorment else, U-boot suited will be created.\n"
		   #else
		    "  efuse list                       - Prints detailed information and all fuses names. \n "
		   #endif
		   );
}

void mvebu_efuse_print_help(void){
	int i;

	printf("\nHelp:\n"
		   "  efuse write 'fuse_name' <value>  - Write <value> to fuse_name corresponding fields. \n"
		   "  efuse read 'fuse_name'           - Read the 'fuse_name' value.\n"
		   "  efuse read                       - Read all available fuses values. \n"
		   "  efuse usage                      - Prints usage syntax, options and descriptions. \n"
		   "  efuse status                     - Prints the modified fuses. \n"
		   "  efuse apply                      - Prints the modified fuses and burn them if permission granted. \n"
		   "  efuse apply verbose              - Prints the modified fuses and burn them if permission granted. Addresses and values are printed\n"
		   "  efuse simulation <on/off>        - Enable/Disable simulation. Simulates registers, no actual fuse register R/W is done.\n"
		   #ifdef LINUX
		   "  efuse --help                     - Prints detailed information and all fuses names. \n"
		   "\nCompilation flags:\n"
		   "  [SIM=y]   - Use gcc. if SIM not used then aarch64-linux-gnu-gcc is used.\n"
		   "  [DEBUG]   - Enable writing to SRAM instead of fuse registers.\n  In addition prints additional data that may help to debug.\n"
		   "  [LINUX=y] - Builds the tool for Linux enviorment else, U-boot suited will be created.\n"
		   #else
		   "  efuse list                       - Prints detailed information and all fuses names. \n "
		   #endif
		   "\nList of all available HD Security Efuse names: \n"
		   "==============================================\n\n");
	for (i = 0; help_info_hd[i] ; i++)
		printf("%s", help_info_hd[i]);
	printf("\nList of all available ID Security Efuse names: \n"
		   "==============================================\n\n");
	for (i = 0; help_menu[i] ; i++)
		printf("%s", help_menu[i]);
}
/**
 * addressee_switch_case set proper destination and title
 * according to given enum addressee.
 *
 *
 * @param dest - Destination resolve.
 * @param title - String according to the resolved detination
 *
 */
static void *addressee_switch_case(enum addressee dest, char *title){
	void *destination;

	switch (dest) {
		case ORIGIN:
			destination = org_destination;
			strncpy(title, "ORIGIN_BASE_ADDRESS", sizeof("ORIGIN_BASE_ADDRESS") + 1);
			break;
		case ALTERED:
			destination = alt_destination;
			strncpy(title, "ALTERED_BASE_ADDRESS", sizeof("ALTERED_BASE_ADDRESS") + 1);
			break;
		case FUSE_REGISTERS:
			if (simulation) {
				destination = sim_destination;
				strncpy(title, "SIMULATION_BASE_ADDRESS", sizeof("SIMULATION_BASE_ADDRESS") + 1);
			}
			else{
				destination = fuse_control_reg_ptr;
				strncpy(title, "FUSE_REGISTERS_BASE_ADDRESS", sizeof("FUSE_REGISTERS_BASE_ADDRESS") + 1);
			}
			break;

		default:
			printf("Error: not a valid addressee.\n");
			destination = NULL;
			break;
	}
	return destination;
}

/**
 *Function description:
 *  Value_write will write to the resource(file or memory array)
 *  according to fd_or_ptr.
 *
 *
 *@param dest - Writing detination.
 * @param value - unsigned bytes array to be written.
 * @param numofbytes - number of bytes to write (valid value : 0 < numofbytes <= 4).
 * @param offset - from destination.
 *
 * @retval (-1) - if error has occured.
 * @retval (0) - else.
 */
static int value_write(enum addressee dest, mv_u8 *value, int numofbytes, mv_u32 offset){
	mv_u32 temp;
	void *destination = NULL;
	bool fd_or_ptr;
	char title[30] = {0};

	if (numofbytes > 4 || numofbytes <= 0) {
		printf("max bytes to be written is 4.\n");
		return -1;
	}

	destination = addressee_switch_case(dest, title);
	if (destination == NULL){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_ADDRESSEE_SWITCH_CASE, dest);
		return -1;
	}

#ifdef LINUX
	if (dest == FUSE_REGISTERS) {
		if (!simulation)
			fd_or_ptr = PTR;
		else
			fd_or_ptr = FD;
	}
	else
		fd_or_ptr = FD;

	if (fd_or_ptr == FD) {
		if ((file_write(*(int *)destination, value, numofbytes, (off_t)offset) == -1)) {
			mvebu_efuse_error_print(1, MVEBU_EFUSE_FILE_WRITE);
			return -1;
		}
	}

#ifdef DEBUG
	temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
	if (dest == FUSE_REGISTERS){
		if (simulation)
			printf("0x@@@@%x <- 0x%x\n", 0x8008 + offset, temp);
		else
			printf("0x%x <- 0x%08x\n", (mv_u32)(print_mem_ptr + offset), temp);
	}
	else
		printf("%s: 0x@@@@%x <- 0x%x\n", title, offset + 0x8008, temp);
#else
	if (verbose){
		temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
		if (dest == FUSE_REGISTERS){
			if (simulation)
			printf("0x@@@@%x <- 0x%x\n", 0x8008 + offset, temp);
		else
			printf("0x%x <- 0x%08x\n", (print_mem_ptr + offset), temp);
		}
	}
#endif

#else/*Uboot*/
	fd_or_ptr = PTR;
#endif

	if (fd_or_ptr == PTR){
		temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
		if (numofbytes < 4) {
			temp = temp & maskarr[numofbytes - 1];
		}
		*(volatile mv_u32 *)(destination + offset) = temp;
	}
#ifndef LINUX
	#ifdef DEBUG
	temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
	if (dest == FUSE_REGISTERS) {
		printf("0x%x <- 0x%x\n", (void *)(destination + offset), temp);
	}
	else{
		printf("%s: 0x%x <- 0x%x\n", title, (void *)(destination + offset), temp);
	}
	#else
	if (verbose){
		temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
		if (dest == FUSE_REGISTERS){
			if (simulation)
				printf("0x@@@@%x <- 0x%x\n", 0x8008 + offset, temp);
			else
				printf("0x%x <- 0x%08x\n", (print_mem_ptr + offset), temp);
		}
	}
	#endif
#endif
	return 0;
}

/**
 *Function description:
 *  Value_read will read the resource(file or memory array)
 *  according to fd_or_ptr.
 *
 *
 * @param dest - Read detination.
 * @param value - unsigned bytes array to be read.
 * @param numofbytes - number of bytes to read (valid value : 0 < numofbytes <= 4).
 * @param offset - from destination.
 *
 * @retval (-1) - if error has occured.
 * @retval (0) - else.
 */
static int value_read(enum addressee dest, mv_u8 *readbytes, int numofbytes, mv_u32 offset){
	mv_u32 temp;
	void *destination = NULL;
	bool fd_or_ptr;
	char title[30] = {0};

	if (numofbytes > 4 || numofbytes <= 0) {
		printf("max bytes to be read is 4, given byte number is %d\n", numofbytes);
		return -1;
	}

	destination = addressee_switch_case(dest, title);
	if (destination == NULL){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_ADDRESSEE_SWITCH_CASE, dest);
		return -1;
	}

	memset(readbytes, 0, numofbytes);/*clear readbytes before usage*/
#ifdef LINUX
	if (dest == FUSE_REGISTERS) {
		if (!simulation)
			fd_or_ptr = PTR;
		else
			fd_or_ptr = FD;
	}
	else
		fd_or_ptr = FD;

	if (fd_or_ptr == FD) {
		if ((file_read(*(int *)destination, readbytes, numofbytes, (off_t)offset) == -1)) {
			mvebu_efuse_error_print(1, MVEBU_EFUSE_FILE_READ);
			return -1;
		}
	}
	#ifdef DEBUG
	temp = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
	printf("%s + 0x%x -> 0x%x\n", title, offset, temp);
	#endif
#else
	fd_or_ptr = PTR;
#endif

	if(fd_or_ptr == PTR){/*fd_or_ptr == ptr*/
		temp = *(volatile mv_u32 *)(destination + offset);
		if (numofbytes < 4) {
			temp = temp & maskarr[numofbytes - 1];
		}
		uint32Touint8arr(readbytes, numofbytes, temp, 0);
	}
#ifndef LINUX
	#ifdef DEBUG
	temp = uint8arrTouint32(value, 0, INDEX_COUNT_UP);
	printf("0x%p -> 0x%x\n", (void *)(destination + offset), temp);
	#endif
#endif
	return 0;
}

/**
 *Function description:
 *id_bit_duplication will apply or remove the bit duplicaiton
 *that is required for the ID fueses.
 *
 * @param apply: 0-REMOVE_DUPLICATION, 1-APPLY_DUPLICATION
 * @param existing_value: the value to apply/remove bit duplication
 * @param fuseindex: fuse index position in securityIDFuse_sysmap[]
 *
 * @retval mv_u32
 */
static mv_u32 id_bit_duplication(bool apply, mv_u32 existing_value, int fuseindex) {
	int key_type_0_index, key_type_3_index;
	bool  is_key_type;

	key_type_0_index = find_id_fuse_index( "key_0_type");
	key_type_3_index = find_id_fuse_index( "key_3_type");
	if (key_type_0_index == (-1) || key_type_3_index == (-1)){
		printf("Couldn't find key type 0 or key type 3 in ID fuse table.\n");
		return 0xffffffff;
	}

	/*This section is used to get rid of bit duplication and correct the read value.*/
	if ((key_type_0_index <= fuseindex) && (fuseindex <= key_type_3_index))
		is_key_type = 1;
	else
		is_key_type = 0;

	if (apply) {
		if (is_key_type) {
			if ((existing_value != 0) && (existing_value != 1) && (existing_value != 2) && (existing_value != 3)) {
				printf("0x%x is an invalid input for %s.\n", existing_value, securityIDFuse_sysmap[fuseindex].name);
				return 0xffffffff;
			}
		}
		else{
			if ((existing_value != 0) && (existing_value != 1)) {
				printf("0x%x is an invalid input for %s.\n", existing_value, securityIDFuse_sysmap[fuseindex].name);
				return 0xffffffff;
			}
		}
		existing_value = ((existing_value >> 1) & 1) * 12 + (existing_value & 1) * 3;/*Aplly duplication formula*/
	}/*if apply*/

	else{/*remove dup*/
		if (is_key_type){
			if ((existing_value != 0) && (existing_value != 3) && (existing_value != 12) && (existing_value != 15)) {
				printf("%x is an invalid input for %s.\n", existing_value, securityIDFuse_sysmap[fuseindex].name);
				return 0xffffffff;
			}
		}
		else{
			if ((existing_value != 0) && (existing_value != 3)) {
				printf("%x is an invalid input for %s.\n", existing_value, securityIDFuse_sysmap[fuseindex].name);
				return 0xffffffff;
			}
		}
		existing_value = ((existing_value >> 2) & 1) * 2 + (existing_value & 1); /*Remove duplication formula*/
	}
	return existing_value;
}

/**
 *
 * @param fuseindex
 *
 * @return shiftcnt: -1 if error has occurred, else positive
 *  	   index location
 */
static int find_bitmask_location(int fuseindex){
	int shiftcnt = 0;

	while (!((securityIDFuse_sysmap[fuseindex].writeBitMask >> shiftcnt) & 0x1)) { /* Count how many shift right is needed */
		shiftcnt++;
		if (shiftcnt > 31){
			printf("infinite loop detected. check bit mask of %s\n", securityIDFuse_sysmap[fuseindex].name);
			return -1;
		}
	}/*while*/
	return shiftcnt;
}



/**
 *Func description:
 * write_fuse_procedure will write value_arr into fuse fields according to fuse_name and the addressee. No tests or checks are committed,
 * this function will write the fuse value according to the fuse registers locations.
 *
 *
 * @param fuse_name
 * @param value_arr - value to be written, given as a mv_u8 array.
 * @param dest - destination memory array. (DB/ Actual fuse registers.)
 *
 * @retval - (-1) - Error.
 * @retval - 0 - else.
 */
static int write_fuse_procedure(char *fuse_name, mv_u8 *value_arr, enum addressee dest){
	int idorhd, fuseindex, shiftcnt, size_to_write = 0, index, i, amount_to_write = 0;
	mv_u32 temp32b = 0,  maskedVal, writeaddr;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	bool has_value;

	detect_fuse(fuse_name, &idorhd, &fuseindex);
	if (idorhd == -1) {
		return -1;
	}

	if (idorhd == 0)/*ID*/ {
		#ifdef DEBUG
		printf("%s:\n", securityIDFuse_sysmap[fuseindex].name);
		#endif

		/*ID fuse read enable*/
		if (dest == FUSE_REGISTERS) {
			if (value_write(dest, (mv_u8 *)id_read_init_vals, 4, 0) == -1) { /* Assuming starting point is EFUSE_CONTROL_REG */
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}
		/*READ CURRENT FUSE VALUE*/
		if (value_read(dest, readbytes, 4, securityIDFuse_sysmap[fuseindex].writeAddr) == -1)
			return -1;
		temp32b = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);

		maskedVal = 0;
		maskedVal = uint8arrTouint32(value_arr, 0, INDEX_COUNT_UP);
		/*MODIFY*/
		temp32b = temp32b & (~securityIDFuse_sysmap[fuseindex].writeBitMask); /*Discards only fuse field bits and keep the rest.*/

		shiftcnt = find_bitmask_location(fuseindex);
		if (shiftcnt == -1)
			return -1;

		maskedVal = id_bit_duplication(APPLY_DUPLICATION, maskedVal, fuseindex);
		if (maskedVal == 0xffffffff)
			return -1;


		temp32b = temp32b | (maskedVal << shiftcnt);/*updated value*/
		/*Write*/
		/*enable ID fuse write*/
		if (dest == FUSE_REGISTERS) {
			if (value_write(dest, (mv_u8 *)id_write_init_vals, 4, 0) == -1) { /* Assuming starting point is AC5_EFUSE_CONTROL_REG */
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}


		uint32Touint8arr(readbytes, 4, temp32b, 0);/*store updated value in readbytes array*/
		if (value_write(dest, readbytes, 4, securityIDFuse_sysmap[fuseindex].writeAddr) == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
			return -1;
		}

		return 0;
	}

	else{/*HD*/
		#ifdef DEBUG
		/*printf("Writing to %s\n", HDsecurityFuse_sysmap[fuseindex].name);*/
		printf("%s:\n", HDsecurityFuse_sysmap[fuseindex].name);
		#endif
		if (verbose)
			printf("\n# %s:\n", HDsecurityFuse_sysmap[fuseindex].name);

		if (dest == FUSE_REGISTERS) {
			temp32b = uint8arrTouint32((mv_u8 *)hd_fuse_en, 0, INDEX_COUNT_UP);
			if (value_write(dest, (mv_u8 *)hd_fuse_en, 4, 0) == -1){/* Write HD fuse en, Assuming starting point is AC5_EFUSE_CONTROL_REG */
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}

		size_to_write = (HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8);/* size in bytes */
		index = 0;
		writeaddr = HDsecurityFuse_sysmap[fuseindex].startAddr;

		while (size_to_write > 0) {
			temp32b = 0;
			has_value = 0;
			if (size_to_write > 4) {
				amount_to_write = 4;
				size_to_write -= 4;
			}
			else{
				amount_to_write = size_to_write;
				size_to_write = 0;
			}

			if (value_write(dest, &value_arr[index], amount_to_write, writeaddr) == -1){
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
			temp32b = uint8arrTouint32(value_arr, index, INDEX_COUNT_UP);
			if (temp32b > 0)
				has_value = 1;

			index += amount_to_write;
			writeaddr += 0x4;

			if (size_to_write <= 0) {
				memset(readbytes, 0, VALUE_MAX_INPUT_SIZE); /*Write zeroes*/
				if (value_write(dest, readbytes, 4, writeaddr) == -1){
					mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
					return -1;
				}
				temp32b = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
				writeaddr += 4;
				readbytes[0] = 0x1;

				if (value_write(dest, readbytes, 1, writeaddr) == -1){/*Write one*/
					mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
					return -1;
				}
				break;
			}

			if (size_to_write > 3) {
				amount_to_write = 3;
				size_to_write -= 3;
			}
			else{
				amount_to_write = size_to_write;
				size_to_write = 0;
			}
			temp32b = 0;
			memset(readbytes, 0, 4);
			for (i = 0; i < amount_to_write; i++) {
				readbytes[i] = value_arr[index];
				temp32b |= (value_arr[index++] << i*8);
			}
			if (value_write(dest, readbytes, amount_to_write, writeaddr) == -1){/*write 3 bytes*/
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
			temp32b = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
			if (temp32b > 0)
				has_value = 1;

			writeaddr += 0x4;
			memset(readbytes, 0, 4);
			readbytes[0] = 0x1;
			if (has_value) {
				if (value_write(dest, readbytes, 1, writeaddr) == -1) { /*write 0x1 to last reg in line*/
					mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
					return -1;
				}
			}
			writeaddr += 0x8; /*move to new line*/
			if (dest == FUSE_REGISTERS){
				if (!simulation){
					sleep(1.5);
					printf("sleep 1.5\n");
				}
			}
		}/* while (size_to_write >= 0) */
	}
	return 0;
}


/**
 * Function description:
 *  This function will read fuse_name value from fuse registers or ALTERED/ORIGIN_FILE_NAME according to addressee.
 *  In addition to printing, the read value will be stored in bytes given array and the number of bytes read is returned.
 *
 * @param fuse_name.
 * @param readbytes - Array to read fuse value into.
 * @param dest - Deignated memory area.
 *
 * @retval (-1) - Error.
 * @retval positive int - Number of bytes read into the given array, readbytes.
 */
static int read_fuse_procedure(char *fuse_name, mv_u8 *readbytes, enum addressee dest){

	int fuseindex, idOrHd, shiftcnt, index, size_to_read, amount_of_bytes = 0;
	mv_u32 temp32b, maskedVal, readaddress;

	memset(readbytes, 0, VALUE_MAX_INPUT_SIZE);/*Reset readbytes before use*/
	detect_fuse(fuse_name, &idOrHd, &fuseindex);
	if (idOrHd == -1) {
		return -1;
	}


	if (idOrHd == 0 ) { /* ID */
		#ifdef DEBUG
		printf("%s:\n", securityIDFuse_sysmap[fuseindex].name);
		#endif

		if (dest == FUSE_REGISTERS){
			if (value_write(dest, (mv_u8 *)id_read_init_vals, 4, 0) == -1) { /* Assuming starting point is EFUSE_CONTROL_REG */
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}

		if (value_read(dest, readbytes, 4, securityIDFuse_sysmap[fuseindex].writeAddr) == -1)
			return -1;
		temp32b = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
		maskedVal = temp32b & securityIDFuse_sysmap[fuseindex].writeBitMask;/*keep only fuse bits*/
		shiftcnt = find_bitmask_location(fuseindex);
		if (shiftcnt == -1)
			return -1;
		maskedVal = maskedVal >> shiftcnt;

		/*This section is used to get rid of bit duplication and correct the read value.*/
		maskedVal = id_bit_duplication(REMOVE_DUPLICATION, maskedVal, fuseindex);
		if (maskedVal == 0xffffffff)
			return -1;


		memset(readbytes, 0, VALUE_MAX_INPUT_SIZE);/*Reset readbytes */
		readbytes[0] = maskedVal;
		return 1;
	} /* ID */

	else{ /* HD */
		#ifdef DEBUG
		printf("%s:\n", HDsecurityFuse_sysmap[fuseindex].name);
		#endif
		if (dest == FUSE_REGISTERS){
			if (value_write(dest, (mv_u8 *)hd_fuse_en, 4, 0) == -1) {/* Assuming starting point is AC5_EFUSE_CONTROL_REG *//*HD_FUSE enable*/
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}

		size_to_read = (HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8);/*Size in bytes*/
		index = 0;
		readaddress = HDsecurityFuse_sysmap[fuseindex].startAddr;
		while (size_to_read > 0) {
			if (size_to_read > 4) {
				amount_of_bytes = 4;
				size_to_read -= 4;
			}
			else{
				amount_of_bytes = size_to_read;
				size_to_read = 0;
			}
			if (value_read(dest, &readbytes[index], amount_of_bytes, readaddress) == -1)
				return -1;

			readaddress += 4;
			index += amount_of_bytes;
			if (size_to_read > 0) {
				if (size_to_read > 3) {
					amount_of_bytes = 3;
					size_to_read -= 3;
				}
				else{
					amount_of_bytes = size_to_read;
					size_to_read = 0;
				}
				if (value_read(dest, &readbytes[index], amount_of_bytes, readaddress) == -1)/* Assuming starting point is AC5_EFUSE_CONTROL_REG */
					return -1;

				readaddress += 0xC;/*skipping over '1' to the next value */
				index += amount_of_bytes;
			}
		}/* while */
		return index;
	}/* HD */
}

#ifdef LINUX
/**
 * Function description:
 *   safe_open_fd opens fd according to path.
 *   creation of the file depends on 'create' bool input
 *   variable.
 *
 * @param path - full file path.
 * @param create - 1:create if doesn't exist. 0:do not create.
 *
 * @retval (-1) - Error.
 * @retval (positive value) - File descriptor number.
 */
static int safe_open_fd(char *path, bool create){
	int fd;

	if (create) {
		fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	}
	else{
		fd = open(path, O_RDWR, S_IRUSR | S_IWUSR);
	}

	if (fd == (-1)) {
		printf("Opening %s failed! \n", path);
		perror("Error Description: ");
		return -1;
	}
	return fd;
}
#endif

static bool check_if_simualtion(void){
	bool sim = 0;

	#ifdef LINUX
		#ifdef SIM
			sim = 1;
		#else
			if (simulation == 1){
				sim = 1;
				return sim;
			}
			if (access(SIM_SRC_FILE_NAME, F_OK | R_OK | W_OK) != -1) {
				sim = 1;
				return sim;
			}

		#endif
	#else /*U-Boot*/
		mv_u32 temp = 0;
		temp = *(volatile mv_u32 *)SIMULATION_FLAG_ADDRESS;
		if (temp == MEMORY_FLAG)
			sim = 1;
	#endif

	return sim;
}

/**
 *Funtion description:
 *Search for prior DB and return answer found/not.
 *
 * @return bool - prior DB search results. 1 - exist , 0 - else.
 */
static bool search_for_prior_db(void){
	bool db_found = 0;
	int  origin_exist = 0 , altered_exist = 0;

	#ifdef LINUX
	origin_exist = access(ORIGIN_FILE_NAME, F_OK | R_OK | W_OK);
	altered_exist = access(ALTERED_FILE_NAME, F_OK | R_OK | W_OK);
	if (origin_exist != -1)
		origin_exist = 1;
	if (altered_exist != -1)
		altered_exist =1;
	#else/*UBoot*/
	mv_u32 temp;

	temp = *(volatile mv_u32 *)ORIGIN_MEM_FLAG;
	if (temp == (mv_u32)MEMORY_FLAG)
		origin_exist = 1;
	temp = *(volatile mv_u32 *)ALTERED_MEM_FLAG;
	if (temp == (mv_u32)MEMORY_FLAG)
		altered_exist = 1;
	#endif
	/*printf("origin_exist = %d\naltered_exist = %d\n", origin_exist, altered_exist);*/
	if ((origin_exist + altered_exist) == 2)
		db_found = 1;

	return db_found;
}

/** setup_DB - initialize DB (origin and altered files/memory arrays) according to environment(Linux/Uboot)
 *             and simulation. The initialization destination is
 *             detrmined by dest.
 *
 *
 * @param dest - memory area destination (DB / Fuse registers)
 *
 * @retval - (-1) - Error.
 * @retval - 0 - else.
 */
static int setup_DB(enum addressee dest){
	int i, amount_read, j;
	bool has_value = 0;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};

	for (i = 0; securityIDFuse_sysmap[i].writeAddr != 0; i++) {/* while not "LAST", iterate over all ID fuses */
		if (read_fuse_procedure(securityIDFuse_sysmap[i].name, readbytes, dest) == -1){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[i].name);
			return -1;
		}

		/*Two writes, one for origin and the other to altered.*/
		if((write_fuse_procedure(securityIDFuse_sysmap[i].name, readbytes, ORIGIN) == -1)\
		|| (write_fuse_procedure(securityIDFuse_sysmap[i].name, readbytes, ALTERED) == -1)){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_WRITE_FUSE_PROCEDURE, securityIDFuse_sysmap[i].name);
			return -1;
		}
	}/*for: securityIDFuse_sysmap[i].writeAddr != 0*/
	for (i = 0; (HDsecurityFuse_sysmap[i].startAddr != 0); i++) { /* while not "LAST", iterate over all HD fuses */
		amount_read = read_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, dest);
		if (amount_read == -1) {
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
			return -1;
		}

		for (j = 0; j < amount_read; j++) {
			if (readbytes[j] != 0)
				has_value = 1;
		}
		if (!has_value)
			continue;/*Prevents redundent hd_fuse 0x1 ending value */
		/*Two writes, one for origin and the other to altered.*/
		if ((write_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, ORIGIN) == -1)\
		|| (write_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, ALTERED)) == -1 ){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_WRITE_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
			return -1;
		}
	}/*for: HDsecurityFuse_sysmap[i]*/
	return 0;
}

/**
 * initialize_resource initializes memory arrays according to
 * arg_count.
 *
 * @param arg_count - number of resources to init. the arguments
 *  				are expected to be enum addressee(int).
 *
 * @return -1 - if error has occurred, else 0.
 */
static int initialize_resource(int arg_count, ...){
	int i, input_addresee[3] = {0};
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	va_list arg_list;
	va_start(arg_list, arg_count);


	switch (arg_count) {

	case 1:
		input_addresee[0] = va_arg(arg_list, int);
		for (i = 0; i < MAX_OFFSET_FROM_CONTROL; i += 4) {
			if (value_write(input_addresee[0], readbytes, 4, i) == -1){
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}
		return 0;
		break;

	case 2:
		input_addresee[0] = va_arg(arg_list, int);
		input_addresee[1] = va_arg(arg_list, int);
		for (i = 0; i < MAX_OFFSET_FROM_CONTROL; i += 4) {
			if ((value_write(input_addresee[0], readbytes, 4, i) == -1)\
				|| (value_write(input_addresee[1], readbytes, 4, i) == -1)){
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}
		break;

	case 3:
		input_addresee[0] = va_arg(arg_list, int);
		input_addresee[1] = va_arg(arg_list, int);
		input_addresee[2] = va_arg(arg_list, int);
		for (i = 0; i < MAX_OFFSET_FROM_CONTROL; i += 4) {
			if ((value_write(input_addresee[0], readbytes, 4, i) == -1)\
				|| (value_write(input_addresee[1], readbytes, 4, i) == -1)\
				|| (value_write(input_addresee[2], readbytes, 4, i) == -1)){
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
				return -1;
			}
		}
		break;

	default:
		printf("arg_count is %d, Invalid input.\n", arg_count);
		return -1;
		break;
	}
	va_end(arg_list);
	return 0;
}


/*detects board type AC5-internal or AC5X-internal*/
static int detect_device(bool *board_is_ac5) {

	/*bool board_is_ac5;   */
	volatile void *pp_space_ptr;
	mv_u32  BoardId;

	#ifdef LINUX
	int memfd;
	if ((memfd = open("/dev/mem", O_RDWR)) < 0){
		perror("open /dev/mem failed.\nError Description: ");
		return -1;
	}

	pp_space_ptr = mmap(NULL,0x1000, PROT_READ | PROT_WRITE , MAP_SHARED , memfd, (uintptr_t)MG0_BASE_ADDRESS); /*Get board id to determine AC5 \ AC5X*/
	if (pp_space_ptr == MAP_FAILED) {
		printf("Could not mmap MG0_BASE_ADDRESS = %x\n", MG0_BASE_ADDRESS);
		return -1;
	}

	#else/*U-Boot*/
	pp_space_ptr = (volatile void *)MG0_BASE_ADDRESS;
	#endif

	BoardId =  *(volatile mv_u32 *)(pp_space_ptr + 0x4c);

	if ((BoardId & 0x000FF000) == 0x000b4000){
		printf("AC5 Detected!\n");
		*board_is_ac5 = 1;
	}
	else if ((BoardId & 0x000FF000) == 0x00098000) {
		printf("AC5X Detected!\n");
		*board_is_ac5 = 0;
	}
	else{
		printf("Failed to detect AC5 or AC5X\n");
		return -1;
	}
	#ifdef LINUX
	close(memfd);
	#endif
	return 0;
}


static void get_fuse_control_reg_ptr(bool board_is_ac5){

#ifdef LINUX
	int memfd;

	if ((memfd = open("/dev/mem", O_RDWR)) < 0){
		perror("open /dev/mem failed.\nError Description: ");
		exit(0);
	}
	if ((board_is_ac5 == 1))
		fuse_control_reg_ptr = mmap(NULL, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, (uintptr_t)AC5_MMAP_ADDR);
	else if ((board_is_ac5 == 0))
		fuse_control_reg_ptr = mmap(NULL, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, (uintptr_t)AC5X_MMAP_ADDR);
	if (fuse_control_reg_ptr == MAP_FAILED) {
		perror("mmap failed.\nError description: ");
		exit(0);
	}
	fuse_control_reg_ptr += 0x8;
	close(memfd);
#else
	if (board_is_ac5 == 1)
		fuse_control_reg_ptr = (void *)AC5_EFUSE_CONTROL_REG;
	else
		fuse_control_reg_ptr = (void *)AC5X_EFUSE_CONTROL_REG;
#endif
	if (!simulation) {
		if (board_is_ac5) {
			print_mem_ptr = AC5_EFUSE_CONTROL_REG;
		}
		else
			print_mem_ptr = AC5X_EFUSE_CONTROL_REG;
	}
}



/**mvebu_efuse_init_DB will setup all relevant resources(file
 *descriptors/ memory pointers). If the resources do not exist
 *this function will create them and initialize them. If the
 *resources exist, this function will open file descriptor and
 *mmap if needed.
 */
int mvebu_efuse_init_DB(void){
	bool prior_db_found;

	#ifdef LINUX
		int sim_file_exist;
	#else
		mv_u32 temp;
	#endif

	prior_db_found = search_for_prior_db();
	if (prior_db_found )
		printf("prior DB was found.\n");
	else
		printf("prior DB was not found.\n");

	simulation = check_if_simualtion();
	if (simulation)
		printf("Simulation mode is ON.\n");
	else
		if(detect_device(&is_ac5) == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_DETECT_DEVICE);
			return -1;
		}

#ifdef LINUX
	if (prior_db_found) {
		originfd = safe_open_fd(ORIGIN_FILE_NAME, DONT_CREATE);
		alteredfd = safe_open_fd(ALTERED_FILE_NAME, DONT_CREATE);
		if ((originfd == -1) || (alteredfd == -1))
			return -1;

		if (simulation) {
			sim_file_exist = access(SIM_SRC_FILE_NAME, F_OK | R_OK | W_OK);
			if (sim_file_exist == -1){
				simulationfd = safe_open_fd(SIM_SRC_FILE_NAME, CREATE);
				if (simulationfd == -1)
					return -1;
				 /*if sim file did not exist and DB did, than DB need to be initialized */
				initialize_resource(3, FUSE_REGISTERS, ORIGIN, ALTERED);
			}/*if (sim_file_exist == -1)*/
			else{
				simulationfd = safe_open_fd(SIM_SRC_FILE_NAME, DONT_CREATE);
				if (simulationfd == -1)
					return -1;
			}
		}
		else{/*not simulation*/
			get_fuse_control_reg_ptr(is_ac5);
		}
		return 0;
	}
	else{/*prior DB was not found*/
		originfd = safe_open_fd(ORIGIN_FILE_NAME, CREATE);
		alteredfd = safe_open_fd(ALTERED_FILE_NAME, CREATE);
		if ((originfd == -1) || (alteredfd == -1))
			return -1;
		/*init DB files*/
		initialize_resource(2, ORIGIN, ALTERED);

		if (simulation){
			sim_file_exist = access(SIM_SRC_FILE_NAME, F_OK | R_OK | W_OK);
			if (sim_file_exist == -1){
				simulationfd = safe_open_fd(SIM_SRC_FILE_NAME, CREATE);
				if (simulationfd == -1)
					return -1;
				initialize_resource(1, FUSE_REGISTERS);
			}/*if (sim_file_exist == -1)*/

			else{
				simulationfd = safe_open_fd(SIM_SRC_FILE_NAME, DONT_CREATE);
				if (simulationfd == -1)
					return -1;
			}

			if (setup_DB(FUSE_REGISTERS) == -1){
				mvebu_efuse_error_print(2, MVEBU_EFUSE_SETUP_DB, FUSE_REGISTERS);
				return -1;
			}

			return 0;
		}/*if(simulation)*/
		else{/*not simulation*/
			get_fuse_control_reg_ptr(is_ac5);
			if (setup_DB(FUSE_REGISTERS) == -1){
				mvebu_efuse_error_print(2, MVEBU_EFUSE_SETUP_DB, FUSE_REGISTERS);
				return -1;
			}
			return 0;
		}
	}

	#else/*U-Boot*/

	if (simulation) {
		temp = *(mv_u32 *)SIMULATION_MEM_FLAG;
		if (temp != MEMORY_FLAG) {
			initialize_resource(1, FUSE_REGISTERS);
			*(mv_u32 *)SIMULATION_MEM_FLAG = MEMORY_FLAG;
		}
	}

	if (prior_db_found)
		return 0;

	else{
		/*clear memory arrays*/
		initialize_resource(2, ORIGIN, ALTERED);
		*(volatile mv_u32 *)ORIGIN_MEM_FLAG = MEMORY_FLAG;
		*(volatile mv_u32 *)ALTERED_MEM_FLAG = MEMORY_FLAG;

		if (!simulation)
			get_fuse_control_reg_ptr(is_ac5);

		if (setup_DB(FUSE_REGISTERS) == -1){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_SETUP_DB, FUSE_REGISTERS);
			return -1;
		}
	}
	#endif
	return 0;

}

void mvebu_efuse_simulation_control(bool on){

	#ifdef LINUX
	if (on)
		simulation = 1; /*This will make mvebu_efuse_init_DB create the simulation file*/

	else{
		if (access(SIM_SRC_FILE_NAME, F_OK | R_OK | W_OK) != -1) { /*This will cause check_if_simulation() return false if SIM is not defined.*/
			remove(SIM_SRC_FILE_NAME);
		}
	}

	#else
	if (on)
		*(volatile mv_u32 *)SIMULATION_FLAG_ADDRESS = MEMORY_FLAG;

	else{
		simulation = 0;
		*(volatile mv_u32 *)SIMULATION_FLAG_ADDRESS = 0;
	}
	#endif
}
/**
 * read and print all fuses values/
 *
 * @return -1 - if error has occured, 0 else.
 */
int mvebu_efuse_read_all(void){
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	int i, j, amount_read;

	printf("\nReading all available fuse values. \n\n");
	printf("\nList of all available ID Security Efuse names: \n");
	printf("==============================================\n");
	for (i = 0; (securityIDFuse_sysmap[i].writeAddr != 0); i++){
		amount_read = read_fuse_procedure(securityIDFuse_sysmap[i].name, readbytes, ALTERED);
		if (amount_read == -1){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[i].name);
			return -1;
		}

		printf("\n%s : 0x", securityIDFuse_sysmap[i].name);
		for (j = amount_read - 1 ; j >= 0 ; j--)
			printf("%x", readbytes[j]);
		printf(".\n\n");
	}

	printf("\nList of all available HD Security Efuse names: \n");
	printf("==============================================\n");
	for (i = 0; (HDsecurityFuse_sysmap[i].startAddr != 0); i++){
		amount_read = read_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, ALTERED);
		if (amount_read == -1){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
			return -1;
		}

		printf("\n%s : 0x", HDsecurityFuse_sysmap[i].name);
		for (j = amount_read -1 ; j >= 0 ; j--)
			printf("%x", readbytes[j]);
		printf(".\n\n");
	}
	return 0;
}


int mvebu_efuse_fuse_read_cmd(char *fusename){
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	int amount_read, j;

	amount_read = read_fuse_procedure(fusename, readbytes, ALTERED);
	if (amount_read == -1){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, fusename);
		return -1;
	}

	printf("\n%s : 0x", fusename);
	for (j = amount_read - 1 ; j >= 0 ; j--)
		printf("%x", readbytes[j]);
	printf(".\n\n");
	return 0;
}


/**
 * input_value_safety_check
 *
 * @param fuse_name
 * @param val - string input value
 * @param bytes - array to store the converted input
 * @param sizeOfbytes - size of bytes
 *
 * @return -1 - if an error has occurred, 0 else
 */
static int input_value_safety_check(char *fuse_name ,char *val, mv_u8 *bytes, int sizeOfbytes){
	int fuseindex, idOrHd, amount_read, i, j;
	char answer;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0}, bitmask, existing_value;

	detect_fuse(fuse_name, &idOrHd, &fuseindex);
	if (idOrHd == -1) {
		return -1;
	}
	if ((strToByteArr(val, bytes, sizeOfbytes) == (-1))) {
		printf("String to number conversion failed.  \n");
		return -1;
	}
	/*********************************Input safety checks*********************************/
	if (idOrHd == 1) {/*HD*/
		if (((strlen(val) - 2) / 2 + (strlen(val) & 1)) > (HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8)) { /*Without 0x prefix*/
			printf("Oversized value detected.\nPlease eneter a valid value sized %d[B], for %s\n", (HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8) ,HDsecurityFuse_sysmap[fuseindex].name);
			return -1;
		}
		if (((strlen(val) - 2)/2 + (strlen(val) & 1)) < (HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8)) {
			printf("Please notice that the byte size of the value entered for %s ,0x%x[B], is smaller than it's logical byte size:%d[B].\n", HDsecurityFuse_sysmap[fuseindex].name,(mv_u32)(((strlen(val) - 2))/2 + (strlen(val) & 1)),(HDsecurityFuse_sysmap[fuseindex].logicalBitSize / 8));
			printf("The value of %s will be padded with zeroes to reach the correct byte size.\n", HDsecurityFuse_sysmap[fuseindex].name);
			answer = 0;
			while (!(answer == 'y' || answer =='Y' || answer == 'n' || answer == 'N')){
				printf("Please enter 'y' for accepting or 'n' for aborting.\n");
				fflush(stdout);
				answer = get_input();
				printf("Character entered is %c \n", answer);
			}
			if (answer == 'n' || answer == 'N') {
				printf("Aborting.\n");
				return -1;
			}
		}
	}
	/*********************************End of input saftey checks*********************************/

	/*********************************Overwriting '1' to '0' test*************************************/
	amount_read = read_fuse_procedure(fuse_name, readbytes, ORIGIN);
	if (amount_read == -1){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, fuse_name);
			return -1;
	}

	for (j = 0; j < amount_read; j++) {
		existing_value = readbytes[j];

		bitmask = 1;
		for (i = 0; i < 8; i++) {/*this section validates that no '1' has changed to '0' before writing. 8 bits are compared*/
			if (i != 0)
				bitmask = bitmask * 2; /*power of 2*/
			if ((existing_value & bitmask) != 0) {/*if '1' existed*/
				if ((existing_value & bitmask) > (bytes[j] & bitmask)) {
					printf("overwriting '1' with '0' is not allowed!\n");
					printf("existing value: 0x%x new value: 0x%x\n", existing_value, bytes[j]);
					return -1;
				}
			}
		}
	/*********************************End of overwriting '1' to '0'*************************************/

	}
	return 0;
}

static int hd_fuse_empty_check(int fuseindex,enum addressee dest){
	int i, amount_read;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};

	if ( (amount_read = read_fuse_procedure(HDsecurityFuse_sysmap[fuseindex].name, readbytes, dest)) == -1){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, HDsecurityFuse_sysmap[fuseindex].name);
		return -1;
	}
	for (i = 0; i < amount_read; i += 4)
		if (uint8arrTouint32(&readbytes[i], 0, INDEX_COUNT_UP))
			return 0;

	return 1;
}

static int fuse_range_lock_check(int fuseindex) {
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	int i = 0;

	if (securityIDFuse_sysmap[fuseindex].idFuseRange == ID_STATUS_REGION_0)
		i = read_fuse_procedure("id_fuse_range_0_lock", readbytes, ORIGIN);
	else if (securityIDFuse_sysmap[fuseindex].idFuseRange == ID_STATUS_REGION_1)
		i = read_fuse_procedure("id_fuse_range_1_lock", readbytes, ORIGIN);
	else if (securityIDFuse_sysmap[fuseindex].idFuseRange == ID_STATUS_REGION_2)
		i = read_fuse_procedure("id_fuse_range_2_lock", readbytes, ORIGIN);
	else{
		printf("Invalid fuse range lock.\n");
		return -1;
	}

	if (i == -1) {
		mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[fuseindex].name); 
		return -1;
	}
	if (uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP) == 1) {
		printf("%s is locked for further burning\n", securityIDFuse_sysmap[fuseindex].name);
		return -1;
	}
	return 0;
}
/**
 * fuse_interactions_check will verify proper fuse setting flow
 * according to predefined fuse relations.
 *
 *
 * @param fusename
 * @param value_arr - given fusename value to be set.
 *
 * @return -1 if an error has occurred, 0 else.
 */
static int fuse_interactions_check(char *fusename, mv_u8 *value_arr){
	int fuseindex, idOrHd, i, j;
	mv_u32 temp32;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	char answer = '\0';
	enum hd_fuse_index{
		JTAGUUID,
		TOKEN,
		PRODUCT_ID,
		OEM_KEY,
		GEN_KEY0,
		GEN_KEY1,
		GEN_KEY2,
		GEN_KEY3};

	enum id_fuse_index{
		SECURE_MODE = 0,
		ENCRYPT_BOOT = 9,
		BOOT_MODE_0_NAND = 10,
		KEY_0_TYPE = 22,
		KEY_1_TYPE = 23,
		KEY_2_TYPE = 24,
		KEY_3_TYPE = 25,
		ID_RANGE_0_LOCK = 26,
		KEY_0_VALID = 47,
		SECURITY_DISABLE = 53
	};

	detect_fuse(fusename, &idOrHd, &fuseindex);/*TODO add safety test print*/
	if (idOrHd == -1)
		return -1;
	else if (idOrHd) { /*HD*/
		switch (fuseindex) {
		default:

			break;
		}
	}
	else{/*ID*/
		i = fuse_range_lock_check(fuseindex);
		if (i != 0) {
			return -1;
		}

		switch (fuseindex) {
		case SECURE_MODE:
			if (value_read(ALTERED, readbytes, 4, securityIDFuse_sysmap[BOOT_MODE_0_NAND].writeAddr) == -1) {
				mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_READ);
				return -1;
			}
			temp32 = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
			temp32 &= BOOT_MODE_MASK_BITS_UNION;/*union of all boot_mode mask bits*/
			if (temp32 == 0) {
				printf("At least one boot mode must be set before setting secure mode.\n");
				return -1;
			}

			if ((i = hd_fuse_empty_check(OEM_KEY, ALTERED))){
				if (i == 1)
					printf("OEM key must be set before setting secure mode.\n ");
				else
				return -1;
			}
			if ( (i = hd_fuse_empty_check(PRODUCT_ID,ALTERED))) {
				if (i == 1)
					printf("Product ID key must be set before setting secure mode.\n ");
				return -1;
			}
			break;


		case ENCRYPT_BOOT:
		/*security_mode enable check*/
		/*if ((j = read_fuse_procedure(securityIDFuse_sysmap[SECURE_MODE].name, readbytes, ALTERED)) == -1){				*/
		/*	mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[(KEY_0_VALID + i)].name);		*/
		/*	return -1;																										*/
		/*}																													*/
		/*temp32 = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);															*/
		/*if (temp32 == 0) {																								*/
		/*	printf("secure_mode must be set before setting encrypt_boot.\n");												*/
		/*	return -1;																										*/
		/*}																													*/


		/*Valid AES key check*/
		for (i = 0; i < 4; i++) {
			if ( (j = read_fuse_procedure(securityIDFuse_sysmap[(KEY_0_VALID + i)].name, readbytes, ALTERED)) == -1) {
				mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[(KEY_0_VALID + i)].name); 
				return -1;
			}
			temp32 = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
			if (temp32 == 1){ /*valid key found*/
				if( read_fuse_procedure(securityIDFuse_sysmap[KEY_0_TYPE + i].name, readbytes, ALTERED) == -1){
					mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[KEY_0_TYPE + i].name);
					return -1;
				}
				temp32 = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
				if ((temp32 == GEN_KEY_TYPE_AES)) {/*AES key found*/
					if (!(j = hd_fuse_empty_check( (GEN_KEY0 + i),  ALTERED)))/*non zero AES key  found*/
						return 0;
					else if (j == -1)
						return -1;
				}
			}
		}
		printf("Before setting encrypt_boot, a generic key with a corresponding key_type set as AES key type\n"
			   "and the corresponding key_valid must be set.\n");
		return -1;
		break;

		case KEY_0_TYPE:
		case KEY_1_TYPE:
		case KEY_2_TYPE:
		case KEY_3_TYPE:

		i = hd_fuse_empty_check((fuseindex - KEY_0_TYPE + GEN_KEY0), ALTERED);
			if (i == -1)
				return -1;
			else if (i == 1){
				printf("%s must be set before setting %s\n", HDsecurityFuse_sysmap[(fuseindex - KEY_0_TYPE + GEN_KEY0)].name, securityIDFuse_sysmap[fuseindex].name);
				return -1;
			}
		return 0;
		break;

		case SECURITY_DISABLE:
			printf("Warning: setting SECURITY_DISABLE fuse will bypass secured boot and disable ARM A55 Crypto extentions PERMANENTLY \n");
			printf("Are you sure you want to set security_disable?\n");
			while (!(answer == 'y' || answer =='Y' || answer == 'n' || answer == 'N')){
				printf("Please enter 'y' for accepting or 'n' for aborting.\n");
				fflush(stdout);
				answer = get_input();
				printf("Character entered is %c \n", answer);
			}
			if (answer == 'n' || answer == 'N') {
				printf("Aborting.\n");
				return -1;
			}
			break;

		default:
		break;
		}/*switch(fuseindex)*/
	}/*ID*/
	return 0;
}



int mvebu_efuse_fuse_write(char *fusename, char *str_value){
	mv_u8 value_arr[VALUE_MAX_INPUT_SIZE] = {0};

	if ((input_value_safety_check(fusename, str_value, value_arr, sizeof(value_arr)))) {
		mvebu_efuse_error_print(2, INPUT_VALUE_SAFETY_CHECK, fusename);
		return -1;
	}

	if (fuse_interactions_check(fusename, value_arr) == -1) {
		printf("fuse_interactions_check failed.\n");
		return -1;
	}

	if (write_fuse_procedure(fusename, value_arr, ALTERED) == -1){
		mvebu_efuse_error_print(2, MVEBU_EFUSE_WRITE_FUSE_PROCEDURE, fusename);
		return -1;
	}

	return 0;
}


/**
 * show_status will print the fuses that were modified and their current and to be burned values.
 *In addition this function will change in the recieved  idIndexChanged hdIndexChanged the fuse indexs that were changed.
 *
 * @param idRegAddChanged - int array sized as the number of ID fuses registers addresses in securityIDFuse_sysmap.
 * @param hdIndexChanged - int array sized as the number of HD fuses in HDsecurityFuse_sysmap.
 *
 * @retval   0 - nothing changed.
 *           1 - somthing changed.
 *         (-1) - error has occured.
 */
static int show_status(bool *idRegAddChanged, bool *hdIndexChanged){
	mv_u8 originbytes[VALUE_MAX_INPUT_SIZE] = {0}, alteredbytes[VALUE_MAX_INPUT_SIZE] = {0};
	int i, j, altered_amount_read, origin_amount_read;
	bool anything_changed = 0;

	memset(idRegAddChanged, 0, NUMBER_OF_ID_REGISTERS * sizeof(bool));
	memset(hdIndexChanged, 0, NUMBER_OF_HD_FUSES * sizeof(bool));
	for (i = 0; securityIDFuse_sysmap[i].writeAddr != 0; i++) {
		origin_amount_read = read_fuse_procedure(securityIDFuse_sysmap[i].name, originbytes, ORIGIN);
		altered_amount_read = read_fuse_procedure(securityIDFuse_sysmap[i].name, alteredbytes, ALTERED);
		if ((origin_amount_read == -1) || (altered_amount_read == -1)){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, securityIDFuse_sysmap[i].name);
			return -1;
		}
		if (originbytes[0] != alteredbytes[0]) {
			printf("%s:\n  origin value: 0x%x\n  altered value: 0x%x\n", securityIDFuse_sysmap[i].name, originbytes[0], alteredbytes[0]);
			idRegAddChanged[(securityIDFuse_sysmap[i].writeAddr - ID_BASE_ADDRESS_OFFSET_FROM_CONTROL)/4] = 1;
			anything_changed = 1;
		}
	}
	for (i = 0; HDsecurityFuse_sysmap[i].startAddr != 0; i++) {
		altered_amount_read = read_fuse_procedure(HDsecurityFuse_sysmap[i].name, originbytes, ORIGIN);
		origin_amount_read = read_fuse_procedure(HDsecurityFuse_sysmap[i].name, alteredbytes, ALTERED);
		if ((origin_amount_read == -1) || (altered_amount_read == -1)){
			mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
			return -1;
		}

		if (memcmp(originbytes, alteredbytes, altered_amount_read) != 0) {
			printf("%s:\norigin value: 0x", HDsecurityFuse_sysmap[i].name);
			for (j = origin_amount_read -1 ; j >= 0 ; j--)
				printf("%x", originbytes[j]);
			printf("\naltered value:0x");
			for (j = altered_amount_read - 1; j >=0 ; j--)
				printf("%x", alteredbytes[j]);
			printf("\n");
			hdIndexChanged[i] = 1;
			anything_changed = 1;
		}
	}
	if (anything_changed == 0)
		printf("Nothing changed.\n");
	return anything_changed;
}

void  mvebu_efuse_status_cmd(void){
	bool idRegAddChanged[NUMBER_OF_ID_REGISTERS] = {0}, hdIndexChanged[NUMBER_OF_HD_FUSES];
	show_status(idRegAddChanged, hdIndexChanged);
}


/**
* mvebu_efuse_apply_changes function will be called once the user has decided that a fuse burn process should take place.
*This function will print all the fuses that should be burned (only altered fuses) and will ask the user to confirm.
*
*@param      verbo - when set, all value_write interactions will
*   			   be printed.(verbose)
*
*@retval     0 - successful exit.
*@retval     1 - unsuccessful exit.
*/
int  mvebu_efuse_apply_changes(bool verbo){

	bool anything_changed = 0, idRegAddChanged[NUMBER_OF_ID_REGISTERS] = {0}, hdIndexChanged[NUMBER_OF_HD_FUSES] = {0};
	int i;
	char answer = 0;
	mv_u8 readbytes[VALUE_MAX_INPUT_SIZE] = {0};
	mv_u32 temp32b = 0;

	if (verbo)
		verbose = 1;

	anything_changed = show_status(idRegAddChanged,hdIndexChanged);
	if(anything_changed){
		printf("Are you sure you want to continue to burning stage?\nThis stage is irreversible!\n");
		while (!(answer == 'y' || answer =='Y' || answer == 'n' || answer == 'N')){
			printf("Please enter 'y' for accepting or 'n' for aborting.\n");
			fflush(stdout);
			answer = get_input();
			printf("Character entered is %c \n", answer);
		}
		if (answer == 'n' || answer == 'N') {
			printf("Aborting.\n");
			return -1;
		}
		else{/*answer == 'y' || answer =='Y' */
			printf("Burning.\n");
			/*Read values from ID registers according to marked address in idRegAddChanges and write as whole register.*/
			for (i = 0; HDsecurityFuse_sysmap[i].startAddr != 0; i++) {/*could replace stopping condition with NUMBER_OF_HD_FUSES*/
				if (hdIndexChanged[i] == 0)
					continue;
				if ((read_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, ALTERED) == -1)){
					mvebu_efuse_error_print(2, MVEBU_EFUSE_READ_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
					return -1;
				}

				if(write_fuse_procedure(HDsecurityFuse_sysmap[i].name, readbytes, FUSE_REGISTERS) == -1){
					mvebu_efuse_error_print(2, MVEBU_EFUSE_WRITE_FUSE_PROCEDURE, HDsecurityFuse_sysmap[i].name);
					return -1;
				}
			}/*for ; HDsecurityFuse_sysmap[i].startAddr != 0;*/
			if( value_write(FUSE_REGISTERS, (mv_u8 *)id_write_init_vals, 4, 0) == -1){
					mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
					return -1;
				}
			for (i = 0; i < NUMBER_OF_ID_REGISTERS; i++) {
				if (idRegAddChanged[i] == 0) {
					continue;
				}
				memset(readbytes, 0, sizeof(readbytes));/*reset readbytes*/
				if( (value_read(ALTERED, readbytes, 4, ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + i*4) == -1) )
					return -1;

				temp32b = 0;
				temp32b = uint8arrTouint32(readbytes, 0, INDEX_COUNT_UP);
				printf("writing ID fuse register 0x%x with value :%x\n", ( ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + i*4), temp32b);


				if( value_write(FUSE_REGISTERS, readbytes, 4, ID_BASE_ADDRESS_OFFSET_FROM_CONTROL + i * 4) == -1){
					mvebu_efuse_error_print(1, MVEBU_EFUSE_VALUE_WRITE);
					return -1;
				}

			}/*for (i = 0; i < NUMBER_OF_ID_REGISTERS; i++)*/
		}/*end of answer == 'y' || answer =='Y'*/
		if (simulation) {
			printf("Burning (Simulation) was successful.\n");
		}
		else{
			printf("Burning to fuse registers was successful.\n");
		}

		#ifdef LINUX
		remove(ORIGIN_FILE_NAME);
		remove(ALTERED_FILE_NAME);
		#else /*UBoot*/
		/*This will cause mvebu_efuse_init_DB to initialize origin and altered memory arrays*/
		*(volatile mv_u32 *)ORIGIN_MEM_FLAG = 0x0;
		*(volatile mv_u32 *)ALTERED_MEM_FLAG = 0x0;
		#endif
		printf("After burning, DB need to update and therefore erased.\n");/*an update for ORIGIN and altered is required after.*/
	}/*if anything_changed*/
	else
		printf("Nothing has changed, hence nothing to burn.\n");
	return 0;
}

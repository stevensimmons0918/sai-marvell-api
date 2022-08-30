/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#include <stdio.h>
#include <stdbool.h>

#ifdef LINUX
	#include <string.h>
	#include <stdint.h>
	#include "fuselist.h"
#else/*UBoot*/
	#include <linux/string.h>
	#include <mvebu/efuse_tool.h>
	#include <config.h>
	#include <common.h>
	#include <command.h>
#endif




#ifdef LINUX
int main(int argc, char *argv[]){
#else
int do_efuse_tool_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char* const argv[]){
#endif
	char *cmd;

	if (argc < 2 || argc > 4){
		mvebu_efuse_print_usage();
		return 0;
	}

	if ( (argc == 2 ) && ( (!(strcmp(argv[1], "--help"))) || (!(strcmp(argv[1], "list")))) ){
		mvebu_efuse_print_help();
		return 0;
	}

	if ((argc == 3) &&  !(strcmp(argv[1], "simulation"))){
		if ((!(strcmp(argv[2], "on"))))
			mvebu_efuse_simulation_control(1);
		else if ((!(strcmp(argv[2], "off"))))
			mvebu_efuse_simulation_control(0);
		else{
			printf("Invalid input for simulation_enable.\n");
			mvebu_efuse_print_usage();
			return -1;
		}
		if(mvebu_efuse_init_DB() == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_INIT_DB);
			return -1;
		}
		return 0;
	}

	cmd = argv[1];
	/*from this stage DB is mandatory*/
	if(mvebu_efuse_init_DB() == -1){
		mvebu_efuse_error_print(1, MVEBU_EFUSE_INIT_DB);
		return -1;
	}

	if ((argc == 2) && (strcmp(cmd, "read") == 0)){/*Read all*/
		if (mvebu_efuse_read_all() == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_READ_ALL);
			return -1;
		}
		return 0;
	}

	if ((argc == 2) && (strcmp(cmd, "status") == 0)) {
		mvebu_efuse_status_cmd();
		return 0;
	}

	if (( (argc == 2) || ((argc == 3) && (strcmp(argv[2], "verbose") == 0))) && (strcmp(cmd, "apply") == 0)) {
		if (argc == 2){
			printf("applying changes \n");
			if ( mvebu_efuse_apply_changes(0) == -1){
			mvebu_efuse_error_print(1,  MVEBU_EFUSE_APPLY_CHANGES);
			return -1;
			}
		}
		else{
			printf("verbose apply activated \n");
			if ( mvebu_efuse_apply_changes(1) == -1){
			mvebu_efuse_error_print(1,  MVEBU_EFUSE_APPLY_CHANGES);
			return -1;
			}
		}
		return 0;
	}

	if ((argc == 3) && (strcmp(cmd, "read") == 0)) {
		printf("fuse_read(%s)\n", argv[2]);
		if (mvebu_efuse_fuse_read_cmd(argv[2]) == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_FUSE_READ_CMD);
			return -1;
		}
		return 0;
	}

	if ((argc == 4) && (strcmp(cmd, "write") == 0)) {
		printf("mvebu_efuse_fuse_write(%s, %s)\n", argv[2], argv[3]);
		if (mvebu_efuse_fuse_write(argv[2], argv[3]) == -1){
			mvebu_efuse_error_print(1, MVEBU_EFUSE_FUSE_WRITE);
			return -1;
		}
		return 0;
	}

	else{
		mvebu_efuse_print_usage();
		return 0;
	}
}


#ifndef LINUX
U_BOOT_CMD(
	efuse_tool,      4,     0,      do_efuse_tool_cmd,
	"efuse - efuse programing tool",
	"\n"
	"  efuse write 'fuse_name' <value>  - Write <value> to fuse_name corresponding fields. \n"
	"  efuse read ['fuse_name']         -If 'fuse_name' is given, read the 'fuse_name' value. Else, read all available fuses values.\n"
	"  efuse list                       - Prints detailed information and all fuses names. \n"
	"  efuse status                     - Prints the modified fuses. \n"
	"  efuse apply                      - Prints the modified fuses and burn them if permission granted. \n"
	"  efuse simulation <value>         - Enable/Disable simulation mode according to <value>. <value> can be 1/0 only.\n"
);
#endif

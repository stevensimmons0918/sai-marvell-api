# Main Makefile for Ros Over linux (BM + WM)
#
SHELL := /bin/bash

BASE_DIR ?= $(shell pwd)
PDL_PATH ?= $(BASE_DIR)/pdl
IDBG_PDL_PATH ?= $(BASE_DIR)/iDbgPdl

.PHONY: all pdl_clean idbg_pdl_clean wm_all
all: wm_all pdl_bm_all idbg_pdl_bm_all

wm_all: pdl_wm_lib idbg_pdl_wm_all

pdl_wm_lib:
	$(MAKE) -f Makefile_wm pdl_lib
	
pdl_bm_lib:
	$(MAKE) -C $(PDL_PATH) bm_lib
	
idbg_pdl_bm_all:
	$(MAKE) -f Makefile_bm 
	
idbg_pdl_wm_all:	
	$(MAKE) -f Makefile_wm idbg_pdl_lib
	
grey_all:
	$(MAKE) -C $(PDL_PATH) grey_all
	$(MAKE) -f Makefile_bm lib GREY_MODE=YES

pdl_clean:
	$(MAKE) -C $(PDL_PATH) bm_clean

idbg_pdl_clean:
	$(MAKE) -f Makefile_bm clean
	
clean: pdl_clean idbg_pdl_clean
	$(MAKE) -f Makefile_wm clean

.PHONY: no_targets__ list
no_targets__:
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"

############################################################
pdl_bm_%::
	$(MAKE) -C $(PDL_PATH) -f Makefile_bm $(patsubst pdl_bm_%,%,$@)
	
pdl_wm_%::
	$(MAKE) -C $(PDL_PATH) -f Makefile_wm $(patsubst pdl_wm_%,%,$@)
	
############################################################

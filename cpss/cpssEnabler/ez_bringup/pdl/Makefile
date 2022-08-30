# Main Makefile for Ros Over linux (BM + WM)
#
SHELL := /bin/bash

.PHONY: all
all: wm_all bm_all

wm_lib:
	$(MAKE) -f Makefile_wm lib
	
grey_all:
	$(MAKE) -f Makefile_bm lib GREY_MODE=yes

bm_lib:
	$(MAKE) -f Makefile_bm lib 
	
wm_clean:
	$(MAKE) -f Makefile_wm clean

bm_clean:
	$(MAKE) -f Makefile_bm clean

clean: wm_clean bm_clean


.PHONY: no_targets__ list
no_targets__:
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"

############################################################
bm_%::
	$(MAKE) -f Makefile_bm $(patsubst bm_%,%,$@)
	
wm_%::
	$(MAKE) -f Makefile_wm $(patsubst wm_%,%,$@)
############################################################


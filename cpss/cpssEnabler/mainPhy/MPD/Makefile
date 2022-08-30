#
SHELL := /bin/bash
DEBUG_MODE?=yes

.PHONY: all doc
all:  bm gm 
#	wm
clean: wm_clean bm_clean gm_clean

bm:
	$(MAKE) -f Makefile_bm DEBUG_FLAG=$(DEBUG_MODE)

gm:
	$(MAKE) -f Makefile_bm GREY_MODE=yes DEBUG_FLAG=$(DEBUG_MODE)
	
wm:
	$(MAKE) -f Makefile_wm DEBUG_FLAG=$(DEBUG_MODE)

wm_clean:
	$(MAKE) -f Makefile_wm DEBUG_FLAG=$(DEBUG_MODE) clean

bm_clean:
	$(MAKE) -f Makefile_bm DEBUG_FLAG=$(DEBUG_MODE) clean

gm_clean:
	$(MAKE) -f Makefile_bm GREY_MODE=yes DEBUG_FLAG=$(DEBUG_MODE) clean

gm_sample:
	$(MAKE) -f Makefile_bm GREY_MODE=yes DEBUG_FLAG=$(DEBUG_MODE) sample

doc:
	doxygen Doxyfile	

.PHONY: no_targets__ list
no_targets__:
list:
	@sh -c "make -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | egrep -v '__\$$|Makefile|make|\%' | sort"


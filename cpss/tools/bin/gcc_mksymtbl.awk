# Just a place Holder for now, we are not
# really using it as we are not building appDemo.exe
# from a linux HOST
# Generate symbol table for VC libraries
# Usage:
#   1. Create library listings:
#      nm -g library1.lib > library.list
#      nm -g library2.lib >> library.list
#      ...
#   2. Generate symbol table:
#      awk -f gcc_mksymtbl.awk library.list > symtable.c
#
/\ T\ / {
	if (ENVIRON["Platform"] != "X64") {
		if (match($(NF), "^_[_a-zA-Z][_a-zA-Z0-9]*$") == 1) {
			sym[substr($(NF),2)] = 1
		}
	} else {
		if (match($(NF), "^[a-zA-Z][_a-zA-Z0-9]*$") == 1) {
			sym[$(NF)] = 1
		}
	}
}
END {
	for (name in sym)
		printf "int %s();\n",name;
	print "struct { const char *name; int (*funcptr)();} __SymbolTable[] = {"
	for (name in sym)
		printf " { \"%s\", %s },\n",name,name
	print " { (void*)0L, (void*)0L }"
	print "};"
}

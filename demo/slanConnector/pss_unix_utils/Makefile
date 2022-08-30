## proposed to be run by unixSlanUtilsBuild.sh
CFLAGS += -Wall -pthread -g
LDFLAGS += -pthread -g

LIBRARY 	= $(OBJ_DIR)slanLib.o 
## Pay attention in FreeBSD must run gmake explicitly to enable gmake extensions 
## like 'shell' command work
CONNECTOR	= slanConnector_$(shell uname)
ifeq ($(shell uname), FreeBSD)
ETH_DRIVER = slanEthPortFreeBsd
else
ETH_DRIVER = slanEthPortLinux
endif

all: $(LIBRARY) $(OBJ_DIR)$(CONNECTOR) $(OBJ_DIR)$(ETH_DRIVER) $(OBJ_DIR)slanTunTapLinux

$(LIBRARY): slanLib.c slanLib.h
	$(CC) $(CFLAGS) -c -o $(LIBRARY) slanLib.c
	
$(OBJ_DIR)$(CONNECTOR): $(OBJ_DIR)$(CONNECTOR).o $(OBJ_DIR)evHandler.o
$(OBJ_DIR)$(CONNECTOR).o: slanConnector.c slanLib.h evHandler.h
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)$(CONNECTOR).o slanConnector.c
$(OBJ_DIR)evHandler.o: evHandler.c evHandler.h
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)evHandler.o evHandler.c

$(OBJ_DIR)$(ETH_DRIVER): $(OBJ_DIR)$(ETH_DRIVER).o $(LIBRARY)
$(OBJ_DIR)$(ETH_DRIVER).o: $(ETH_DRIVER).c slanLib.h
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)$(ETH_DRIVER).o $(ETH_DRIVER).c

slanTunTapLinux: $(OBJ_DIR)slanTunTapLinux.o $(LIBRARY) $(OBJ_DIR)evHandler.o
slanTunTapLinux.o: slanTunTapLinux.c $(LIBHEADERS)
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)slanTunTapLinux.o slanTunTapLinux.c

clean:
## TBD: if OBJ_DIR != current dir
	rm -f ${OBJ_DIR}*.o *~ $(CONNECTOR) $(ETH_DRIVER) slanTunTapLinux

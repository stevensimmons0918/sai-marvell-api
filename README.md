# Build cmds, from sai-marvell-api/build:

## Simualtion with DEBUG:

> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 cpss saiPreproc xpSaiApp saiShell_py

## Hardware with debug:

> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=aarch64 cpss saiPreproc xpSaiApp saiShell_py
> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=x86_64 cpss saiPreproc xpSaiApp saiShell_py
> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=a385 cpss saiPreproc xpSaiApp saiShell_py

## WM binary testing: 

> ./dist/xpSaiApp -g FALCON128 -u

## Setting slan tap interfaces on VM: 

> cd sai_cpss/demo/slanConnector/pss_unix_utils/
> make

> ./slanConnector_Linux -D
> sudo ./slanTunTapLinux -D -n slan00
> sudo ./slanTunTapLinux -D -n slan01
> sudo ./slanTunTapLinux -D -n slan02
> sudo ./slanTunTapLinux -D -n slan03
> sudo ./slanTunTapLinux -D -n slan04
> sudo ./slanTunTapLinux -D -n slan05
> sudo ifconfig tap0 up
> sudo ifconfig tap1 up
> sudo ifconfig tap2 up
> sudo ifconfig tap3 up
> sudo ifconfig tap4 up
> sudo ifconfig tap5 up

## Running CI apptest:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> <path to sai_cpss\> sai feature all 1

*sample scapy commands:*
> \>>> p=Ether(dst="00:e1:e2:33:44:ef",src="c2:00:57:75:00:00")\/IP(dst="2.2.2.2",src="1.1.1.2")\/ICMP()

> \>>> sendp(p,iface="tap0")

## Run fujitsu testList for ALDRIN2XLFL:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> blade/sai-marvell-api/ sai-fl feature all 0

## Run single test with debug flag for ALDRIN2XLFL:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> blade/sai-marvell-api/ sai-fl feature saiCrmAclTableTest 1

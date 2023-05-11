# Build cmds, from sai-marvell-api/build:

## Simualtion with DEBUG:

> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 cpss saiPreproc xpSaiApp saiShell_py <br />

## Hardware with debug:

> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=aarch64 cpss saiPreproc xpSaiApp saiShell_py <br />
> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=x86_64 cpss saiPreproc xpSaiApp saiShell_py <br />
> ./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 TARGET=a385 cpss saiPreproc xpSaiApp saiShell_py <br />

## WM binary testing: 

> ./dist/xpSaiApp -g FALCON128 -u <br />

## Setting slan tap interfaces on VM: 

> cd sai-marvell-api/demo/slanConnector/pss_unix_utils/ <br />
> make <br />

> ./slanConnector_Linux -D <br />
> sudo ./slanTunTapLinux -D -n slan00 <br />
> sudo ./slanTunTapLinux -D -n slan01 <br />
> sudo ./slanTunTapLinux -D -n slan02 <br />
> sudo ./slanTunTapLinux -D -n slan03 <br />
> sudo ./slanTunTapLinux -D -n slan04 <br />
> sudo ./slanTunTapLinux -D -n slan05 <br />
> sudo ifconfig tap0 up <br />
> sudo ifconfig tap1 up <br />
> sudo ifconfig tap2 up <br />
> sudo ifconfig tap3 up <br />
> sudo ifconfig tap4 up <br />
> sudo ifconfig tap5 up <br />

## Running CI apptest:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> <path to sai_cpss\> sai feature all 1 <br />

*sample scapy commands:* <br />
> \>>> p=Ether(dst="00:e1:e2:33:44:ef",src="c2:00:57:75:00:00")\/IP(dst="2.2.2.2",src="1.1.1.2")\/ICMP() <br />

> \>>> sendp(p,iface="tap0") <br />

## Run fujitsu testList for ALDRIN2XLFL:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> blade/sai-marvell-api/ sai-fl feature all 0 <br />

## Run single test with debug flag for ALDRIN2XLFL:

> sudo python apptest.py 127.0.0.1 <user-name\> <passwdxx\> blade/sai-marvell-api/ sai-fl feature saiCrmAclTableTest 1 <br />

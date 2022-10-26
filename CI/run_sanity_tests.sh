#!/usr/bin/env bash
set -x

# Prepare simulation
SIM_PATH='demo/slanConnector/pss_unix_utils'
./$SIM_PATH/slanConnector_Linux -D
./$SIM_PATH/slanTunTapLinux -D -n slan00
./$SIM_PATH/slanTunTapLinux -D -n slan01
./$SIM_PATH/slanTunTapLinux -D -n slan02
./$SIM_PATH/slanTunTapLinux -D -n slan03
./$SIM_PATH/slanTunTapLinux -D -n slan04
./$SIM_PATH/slanTunTapLinux -D -n slan05

# Disable IPv6 on all interfaces
sysctl -w net.ipv6.conf.all.autoconf=0
sysctl -w net.ipv6.conf.all.disable_ipv6=1

# Set Tap interfaces to up state
ifconfig tap0 up
ifconfig tap1 up
ifconfig tap2 up
ifconfig tap3 up
ifconfig tap4 up
ifconfig tap5 up

# Start OpenSSH
/usr/sbin/sshd -D &

# Run tests
mkdir ../esal-l2swdrvr-sai
mkdir ../esal-l2swdrvr-sai/iniFiles
cp cpss/simulation/registerFiles/Fujitsu/aldrin_fl.registers_default_val.txt ../esal-l2swdrvr-sai/iniFiles/
cp cpss/simulation/registerFiles/Fujitsu/aldrin_fl.registers_additional_def_val.txt ../esal-l2swdrvr-sai/iniFiles/
cd apptest/
mkdir TestResults
python apptest.py 127.0.0.1 root password /var/sai-marvell-api/ sai-fl sanity all 1

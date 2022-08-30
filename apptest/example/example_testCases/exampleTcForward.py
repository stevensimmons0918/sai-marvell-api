# coding=utf-8
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
'ingressPcap' : 'testPkt_Vlan_500_DA_deadbeef.pcap',
'tcName' : 'vxlanTc10',
'description' : 'TC to verify that switch should Originate the VxLan Tunnel and route packet to appropriate destination port if VxLan TO is configured and Lookup to Ipv4Host table gets success.',
'ingressPort' : ['8'],
'egressPort' : ['9'],
'vlan' : '500',
'pktAction' : 'FORWARD',
'count': 2,         # number of expected data
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
xps
vlan
vlan_create 0 500
vlan_add_interface 0 500 8 1
vlan_add_interface 0 500 9 1
vlan_show 0 500
back
fdb
fdb_add_entry 0 500 00:00:de:ad:be:ef 1 0 0 1 9 1234
back
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
xps
vlan
vlan_remove_interface 0 500 8
vlan_remove_interface 0 500 9
vlan_destroy 0 500
back
fdb
fdb_remove_entry 0 500 00:00:de:ad:be:ef 9
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
	   'expect1':'''00 00 de ad be ef 02 04 05 05 11 11 81 00 01 f4
 08 00 45 00 00 00 00 00 40 00 ff 2f c0 47 64 c8
 de 0b 32 a8 46 0b 00 00 08 00 45 00 00 34 55 db
 40 00 3f 06 81 d0 42 3b 6f be ac 1c 06 03 9d 48
 00 16 30 7f 72 e8 f9 56 5f 98 80 10 19 20 32 28
 00 00 00 01 01 08 0a 06 b1 26 14 00 3d 04 a5 55
 55 55 55''',
	'expect2':'cpuCode=563'
}

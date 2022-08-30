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
'pktAction' : 'DROP',
'count': 2,   # number of expected data
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
fdb_add_entry 0 500 00:00:de:ad:be:ef 0 0 0 1 9 1234
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
back
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
	   'expect1':'''''',
	'expect2':''
}

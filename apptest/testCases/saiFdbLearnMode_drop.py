import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiFdbLearnMode.py',
    'description'    : 'TC to verify FDB Learn mode Trap',
    'ingressPort'    : ['29'],
    'egressPort'     : ['30'],
    'vlan'           : '75',
    'pktAction'      : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'count'          : 0              # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
bridge
sai_set_bridge_port_attribute 16325548649218077 SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE 0
back
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
bridge
sai_set_bridge_port_attribute 16325548649218077 SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE 2
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP()/scapy_t.TCP(sport=2471,dport=2472, flags="SA")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}


import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiFdbDynamicFwd_II',
    'description' : 'TC to verify the dynamic learning (part-2)',
    'ingressPort' : ['30'],
    'egressPort' : ['29'],
    'vlan'       : '75',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap1'],
    'count' : 1 ,            # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:11",dst="00:00:11:00:11:00")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()
#packet_info = scapy_t.Ether(src="00:00:11:00:11:11",dst="00:00:11:00:11:00")/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap1':[<Ether  dst=00:00:11:00:11:00 src=00:00:11:00:11:11 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0x7ce7 src=127.0.0.1 dst=127.0.0.1
'''
}

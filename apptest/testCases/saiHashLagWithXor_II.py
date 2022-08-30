import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Ipv4 Packet',
    'tcName' : 'saiHashLagWithXor',
    'description' : 'Verify xor load balance for lag hash',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,             # expected data count
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
home
sai
fdb
sai_remove_fdb_entry 9288674231451648 [00:00:01:11:22:03] $vlan10

back
vlan
sai_remove_vlan $vlan10

back
bridge
sai_remove_bridge_port $brgport

back
lag
sai_remove_lag_member $lag_member_1
sai_remove_lag_member $lag_member_2
sai_remove_lag $lag_id_1

back
switch
sai_set_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM 0

back
hash
sai_set_hash_attribute 7881299347898368 SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST 14:17:18:19
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:03")/scapy_t.Dot1Q(vlan =10)/scapy_t.IP(dst="10.10.10.1", src="20.1.1.7",ttl=45)




#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':''
}


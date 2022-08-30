import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Ipv4 Packet',
    'tcName' : 'saiHashLagWithCrc',
    'description' : 'Verify crc load balance for lag hash',
    'ingressPort' : ['29'],
    'egressPort' : ['28'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap0'],
    'count' : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

back
lag
sai_create_lag 9288674231451648 1 > lag_id_1
sai_create_lag_member 9288674231451648 281474976710684 $lag_id_1 > lag_member_1
sai_create_lag_member 9288674231451648 281474976710686 $lag_id_1 > lag_member_2

back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg

back
bridge
sai_create_bridge_port 9288674231451648 0 $lag_id_1 $brg SAI_BRIDGE_PORT_TYPE_PORT 0 0 > brgport

back
vlan
sai_create_vlan 9288674231451648 10 > vlan10
sai_create_vlan_member 9288674231451648 $vlan10 16325548649218077 > vmem29
sai_create_vlan_member 9288674231451648 $vlan10 $brgport > vmem2830

back
fdb
sai_create_fdb_entry 9288674231451648 [00:00:01:11:22:03] $vlan10 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 $brgport SAI_FDB_ENTRY_TYPE_STATIC

back
hash
sai_set_hash_attribute 7881299347898368 SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST 0:1

'''
#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:03")/scapy_t.Dot1Q(vlan =10)/scapy_t.IP(dst="10.10.10.1", src="20.1.1.2",ttl=45)



#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':''
}


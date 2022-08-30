#!/usr/bin/env python
#  saiShellSwitch.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellSwitch.py
#

import sys
import time
import os
import re
import readline

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd
import xpShellGlobals
import saiShellGlobals
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for saiShellSwitch operations
#/**********************************************************************************/

class saiShellSwitchObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_switch
    #/*********************************************************/
    def do_sai_create_switch(self, arg):

        ''' sai_create_switch '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' type, switch_profile_id, switch_hardware_info, init_switch'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'type','switch_profile_id','switch_hardware_info','init_switch' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[2] = args[2].replace(".",":").replace(",",":")
		    args[2] = args[2].strip(',').strip("'")
		    if args[2] != "[]":
			    postList = args[2].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    switch_hardware_info_ptr = new_sai_s8_list_tp()
		    switch_hardware_info_ptr.count = listLen
		    switch_hardware_info_list_ptr = new_arr_sai_s8(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s8_setitem(switch_hardware_info_list_ptr, ix, int(postList[ix]))
		    switch_hardware_info_ptr.list = switch_hardware_info_list_ptr
		    ret = sai_create_switch_default(eval(args[0]),int(args[1]),switch_hardware_info_ptr,int(args[3]) )
		    delete_sai_s8_list_tp(switch_hardware_info_ptr)
		    delete_arr_sai_s8(switch_hardware_info_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' type, fdb_event_notify, qos_default_tc, qos_tc_and_color_to_dot1p_map, switch_profile_id, fdb_unicast_miss_packet_action, macsec_object_id, qos_dscp_to_color_map, fdb_multicast_miss_packet_action, qos_tc_to_queue_map, fdb_aging_time, port_state_change_notify, tunnel_loopback_packet_action, qos_dot1p_to_color_map, ecmp_default_hash_algorithm, lag_default_hash_algorithm, qos_dot1p_to_tc_map, qos_dscp_to_tc_map, qos_tc_and_color_to_dscp_map, switch_shell_enable, ecmp_default_hash_seed, max_learned_addresses, vxlan_default_port, fdb_broadcast_miss_packet_action, switch_hardware_info, egress_acl, ingress_acl, init_switch, vxlan_default_router_mac, src_mac_address, lag_default_hash_seed'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'type','fdb_event_notify','qos_default_tc','qos_tc_and_color_to_dot1p_map','switch_profile_id','fdb_unicast_miss_packet_action','macsec_object_id','qos_dscp_to_color_map','fdb_multicast_miss_packet_action','qos_tc_to_queue_map','fdb_aging_time','port_state_change_notify','tunnel_loopback_packet_action','qos_dot1p_to_color_map','ecmp_default_hash_algorithm','lag_default_hash_algorithm','qos_dot1p_to_tc_map','qos_dscp_to_tc_map','qos_tc_and_color_to_dscp_map','switch_shell_enable','ecmp_default_hash_seed','max_learned_addresses','vxlan_default_port','fdb_broadcast_miss_packet_action','switch_hardware_info','egress_acl','ingress_acl','init_switch','vxlan_default_router_mac','src_mac_address','lag_default_hash_seed' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[24] = args[24].replace(".",":").replace(",",":")
		    args[24] = args[24].strip(',').strip("'")
		    if args[24] != "[]":
			    postList = args[24].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    switch_hardware_info_ptr = new_sai_s8_list_tp()
		    switch_hardware_info_ptr.count = listLen
		    switch_hardware_info_list_ptr = new_arr_sai_s8(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s8_setitem(switch_hardware_info_list_ptr, ix, int(postList[ix]))
		    switch_hardware_info_ptr.list = switch_hardware_info_list_ptr
		    args[28] = args[28].replace(".",":").replace(",",":")
		    postList = args[28].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    vxlan_default_router_mac_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    vxlan_default_router_mac_ptr.mac[ix] =  int(postList[ix], 16)
		    args[29] = args[29].replace(".",":").replace(",",":")
		    postList = args[29].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    src_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_switch_custom(eval(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]),eval(args[5]),int(args[6]),int(args[7]),eval(args[8]),int(args[9]),int(args[10]),int(args[11]),eval(args[12]),int(args[13]),eval(args[14]),eval(args[15]),int(args[16]),int(args[17]),int(args[18]),int(args[19]),int(args[20]),int(args[21]),int(args[22]),eval(args[23]),switch_hardware_info_ptr,int(args[25]),int(args[26]),int(args[27]),vxlan_default_router_mac_ptr,src_mac_address_ptr,int(args[30]))
		    delete_sai_s8_list_tp(switch_hardware_info_ptr)
		    delete_arr_sai_s8(switch_hardware_info_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_switch_attribute
    #/*********************************************************/
    def do_sai_set_switch_attribute(self, arg):

	'''sai_set_switch_attribute: Enter[ switch_id, attr_id, attr_value ]'''

	varStr = 'switch_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_L3_NEIGHBOR_TABLE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_NUMBER_OF_SUPPORTED_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_BFD_SESSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SEGMENTROUTE_TLV_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_ACL_RANGE_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_CAPABILITY'):
			args[2]= int(args[2])
			attr_value_ptr.aclcapability= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FAST_API_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PACKET_DMA_MEMORY_POOL_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_BFD_SESSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NV_STORAGE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEXTHOP_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_OPER_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MIN_BFD_RX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TPID_INNER_VLAN'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_L3_ROUTE_TABLE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_DEFAULT_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_LOAD_METHOD'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_DOWNLOAD_BROADCAST'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_PROFILE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MCAST_SNOOPING_CAPABILITY'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PFC_TC_DLD_INTERVAL'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.maplist.count = listLen
			list_ptr = new_arr_sai_map(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_map_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.maplist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_map(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_ACL_ACTION_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_DOWNLOAD_EXECUTE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_REGISTER_READ'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_MINOR_VERSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MACSEC_OBJECT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_TABLE_GROUP_MAXIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PFC_TC_DLD_INTERVAL_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PFC_DLR_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_SNAT_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SYSTEM_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_HASH_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_STAGE_EGRESS'):
			args[2]= int(args[2])
			attr_value_ptr.aclcapability= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_DEFAULT_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_TABLE_GROUP_MINIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVERAGE_TEMP'):
			args[2]= int(args[2])
			attr_value_ptr.s32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MIN_PLANNED_RESTART_INTERVAL'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_AGING_TIME'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECN_ECT_THRESHOLD_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_MIRROR_SESSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPMC_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_HASH'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_RESTART_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.aclresource.count = listLen
			list_ptr = new_arr_sai_acl_resource(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_acl_resource_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.aclresource.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_acl_resource(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_HASH'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NAT_ZONE_COUNTER_OBJECT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_SAMPLED_MIRROR_SESSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TUNNEL_LOOPBACK_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_NUMBER_OF_TEMP_SENSORS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_UNINIT_DATA_PLANE_ON_REMOVAL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_WARM_RECOVER'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_CRC_RECALCULATION_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE_GROUP'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.aclresource.count = listLen
			list_ptr = new_arr_sai_acl_resource(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_acl_resource_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.aclresource.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_acl_resource(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_LAGS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_HASH_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PORT_CONNECTOR_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_OBJECT_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PROPOGATE_PORT_STATE_FROM_LINE_TO_SYSTEM_PORT_SUPPORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_USER_TRAP_ID_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SEGMENTROUTE_MAX_SID_DEPTH'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_VERIFY_AND_INIT_SWITCH'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_STATUS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_PROTECTED_OBJECT_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_TEMP'):
			args[2]= int(args[2])
			attr_value_ptr.s32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TAM_OBJECT_ID'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PRE_SHUTDOWN'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_SHUTDOWN_REQUEST_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEXTHOP_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_HASH_IPV4'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_HASH_IPV4'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MPLS_EXP_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_CPU_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NAT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_MPLS_EXP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_HARDWARE_ACCESS_BUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_MEMBERS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_DOUBLE_NAT_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_STP_INSTANCE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_SYSTEM_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s8list.count = listLen
			list_ptr = new_arr_sai_s8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s8_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s8list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s8(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PFC_TC_DLR_INTERVAL_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MIRROR_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MIN_BFD_TX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_VXLAN_DEFAULT_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_MAJOR_VERSION'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_ACTIVE_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_REGISTER_WRITE'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s8list.count = listLen
			list_ptr = new_arr_sai_s8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s8_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s8list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s8(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_CRC_CHECK_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_IPV6_BFD_SESSION_OFFLOAD_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TAM_EVENT_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_BROADCAST_STOP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FABRIC_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PORT_MAX_MTU'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_L2MC_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_EGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_IPV4_BFD_SESSION_OFFLOAD_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TEMP_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FIRMWARE_LOAD_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_MAX_SYSTEM_CORES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_MEMBERS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_INGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_NUM_LOSSLESS_QUEUES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SYSTEM_PORT_CONFIG_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.sysportconfiglist.count = listLen
			list_ptr = new_arr_sai_system_port_config(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_system_port_config_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.sysportconfiglist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_system_port_config(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FDB_TABLE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SWITCHING_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_QOS_MPLS_EXP_TO_COLOR_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_INIT_SWITCH'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_VXLAN_DEFAULT_ROUTER_MAC'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SRC_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_FAILOVER_MODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_TPID_OUTER_VLAN'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ACL_STAGE_INGRESS'):
			args[2]= int(args[2])
			attr_value_ptr.aclcapability= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEIGHBOR_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_FAILOVER_CONFIG_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_RESTART_WARM'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_DNAT_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PLATFROM_CONTEXT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_FABRIC_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY'):
			args[2]= int(args[2])
			attr_value_ptr.ptr= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_PFC_TC_DLR_INTERVAL'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.maplist.count = listLen
			list_ptr = new_arr_sai_map(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_map_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.maplist.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_map(list_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_AVAILABLE_IPV4_ROUTE_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_SWITCH_ATTR_SUPPORTED_EXTENDED_STATS_MODE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_switch_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_switch_attribute
    #/*********************************************************/
    def do_sai_get_switch_attribute(self, arg):

	'''sai_get_switch_attribute: Enter[  switch_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' switch_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_switch_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_switch_stats
    #/*********************************************************/
    def do_sai_get_switch_stats(self, arg):

	'''sai_get_switch_stats: Enter[  switch_id, number_of_counters ]'''

	varStr = ' switch_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 1)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_switch_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 1):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)
    #/*********************************************************/
    # command for sai_clear_switch_stats
    #/*********************************************************/
    def do_sai_clear_switch_stats(self, arg):

	'''sai_clear_switch_stats: Enter[  switch_id, number_of_counters ]'''

	varStr = ' switch_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_clear_switch_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_switch_stats_ext
    #/*********************************************************/
    def do_sai_get_switch_stats_ext(self, arg):

	'''sai_get_switch_stats_ext: Enter[  switch_id, number_of_counters, mode ]'''

	varStr = ' switch_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','number_of_counters','mode' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(r'[;,\s]\s*',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 2)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 2)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 2):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_switch_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 2):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)

    #/*********************************************************/
    # command for sai_remove_switch
    #/*********************************************************/
    def do_sai_remove_switch(self, arg):

	'''sai_remove_switch: Enter[  switch_id ]'''

	varStr = ' switch_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_switch(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for alter_create_mode
    #/*********************************************************/
    def do_alter_create_mode(self, arg):

        '''alter_create_mode: Enter [ 'create_mode' ] '''

        varStr = 'create_mode'
        varList = varStr.replace(' ', '').split(',')
        optArg = [ 'create_mode' ]
        args = ['0']*len(varList)
        totNumArgs = len(varList)

        argsList = re.split(',| ',arg)
        args = argsList
        if (totNumArgs > 0 and argsList[0] == ''):
            print('Invalid input, Enter [ %s ]' % varStr)
        else:
            xpShellGlobals.cmdRetVal = long(int(args[0]))
            saiShellGlobals.createMode = int(args[0])
#!/usr/bin/env python
#  saiShellAcl.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellAcl.py
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
# The class object for saiShellAcl operations
#/**********************************************************************************/

class saiShellAclObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_acl_table_group_member
    #/*********************************************************/
    def do_sai_create_acl_table_group_member(self, arg):

        ''' sai_create_acl_table_group_member '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, acl_table_id, acl_table_group_id, priority'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','acl_table_id','acl_table_group_id','priority' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_table_group_member_default(int(args[0]),int(args[1]),int(args[2]),int(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, acl_table_id, acl_table_group_id, priority'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','acl_table_id','acl_table_group_id','priority' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_table_group_member_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_acl_range
    #/*********************************************************/
    def do_sai_create_acl_range(self, arg):

        ''' sai_create_acl_range '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, type, limit'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type','limit' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_range_default(int(args[0]),eval(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, type, limit'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type','limit' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_range_custom(int(args[0]),eval(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_acl_table_group
    #/*********************************************************/
    def do_sai_create_acl_table_group(self, arg):

        ''' sai_create_acl_table_group '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, acl_stage, acl_bind_point_type_list, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','acl_stage','acl_bind_point_type_list','type' ]
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
		    acl_bind_point_type_list_ptr = new_sai_s32_list_tp()
		    acl_bind_point_type_list_ptr.count = listLen
		    acl_bind_point_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_bind_point_type_list_list_ptr, ix, int(postList[ix]))
		    acl_bind_point_type_list_ptr.list = acl_bind_point_type_list_list_ptr
		    ret = sai_create_acl_table_group_default(int(args[0]),eval(args[1]),acl_bind_point_type_list_ptr,eval(args[3]))
		    delete_sai_s32_list_tp(acl_bind_point_type_list_ptr)
		    delete_arr_sai_s32(acl_bind_point_type_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, acl_stage, acl_bind_point_type_list, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','acl_stage','acl_bind_point_type_list','type' ]
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
		    acl_bind_point_type_list_ptr = new_sai_s32_list_tp()
		    acl_bind_point_type_list_ptr.count = listLen
		    acl_bind_point_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_bind_point_type_list_list_ptr, ix, int(postList[ix]))
		    acl_bind_point_type_list_ptr.list = acl_bind_point_type_list_list_ptr
		    ret = sai_create_acl_table_group_custom(int(args[0]),eval(args[1]),acl_bind_point_type_list_ptr,eval(args[3]))
		    delete_sai_s32_list_tp(acl_bind_point_type_list_ptr)
		    delete_arr_sai_s32(acl_bind_point_type_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_acl_entry
    #/*********************************************************/
    def do_sai_create_acl_entry(self, arg):

        ''' sai_create_acl_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, table_id,  priority'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','table_id','priority' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_entry_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, field_out_ports, action_egress_samplepacket_enable, action_mirror_ingress, action_set_policer, admin_state, field_l4_src_port, field_ip_protocol, field_l4_dst_port, field_dscp, field_ipv6_next_header, action_mirror_egress, priority, field_dst_mac, field_in_port, field_acl_ip_type, field_src_ip, field_dst_ipv6, field_outer_vlan_id, field_dst_ip, action_counter, field_tc, field_tos, table_id, field_acl_range_type, field_icmp_type, field_src_ipv6, field_src_mac, field_tcp_flags, field_icmp_code, field_ether_type, field_out_port, action_packet_action, action_ingress_samplepacket_enable, field_icmpv6_type, action_redirect, field_in_ports, field_icmpv6_code'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','field_out_ports','action_egress_samplepacket_enable','action_mirror_ingress','action_set_policer','admin_state','field_l4_src_port','field_ip_protocol','field_l4_dst_port','field_dscp','field_ipv6_next_header','action_mirror_egress','priority','field_dst_mac','field_in_port','field_acl_ip_type','field_src_ip','field_dst_ipv6','field_outer_vlan_id','field_dst_ip','action_counter','field_tc','field_tos','table_id','field_acl_range_type','field_icmp_type','field_src_ipv6','field_src_mac','field_tcp_flags','field_icmp_code','field_ether_type','field_out_port','action_packet_action','action_ingress_samplepacket_enable','field_icmpv6_type','action_redirect','field_in_ports','field_icmpv6_code' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    nameList = args[1::2]
		    dataList = args[2::2]
		    for ix in range(0,len(nameList),1):
			    if '-' not in nameList[ix]:
				    print 'Invalid Input'
		    for ix in range(0,len(dataList),1):
			    if '-' in dataList[ix]:
				    print 'Invalid Input'
		    for ix in range(0,len(dataList),1):
			    nameList[ix] = nameList[ix].replace('-','')
		    flag = 0
		    acl_ptr_1 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_out_ports" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_1.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    acl_ptr_1.data.objlist.count = listLen
				    list_ptr = new_arr_sai_object(listLen)
				    for ix in range(0, listLen, 1):
					    arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
				    acl_ptr_1.data.objlist.list = list_ptr
		    if flag == 0:
			    acl_ptr_1.enable = False
		    flag = 0

		    acl_ptr_2 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_egress_samplepacket_enable" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_2.enable = True
				    acl_ptr_2.parameter.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_2.enable = False
		    flag = 0

		    acl_ptr_3 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_mirror_ingress" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_3.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    acl_ptr_3.parameter.objlist.count = listLen
				    list_ptr = new_arr_sai_object(listLen)
				    for ix in range(0, listLen, 1):
					    arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
				    acl_ptr_3.parameter.objlist.list = list_ptr
		    if flag == 0:
			    acl_ptr_3.enable = False
		    flag = 0

		    acl_ptr_4 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_set_policer" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_4.enable = True
				    acl_ptr_4.parameter.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_4.enable = False
		    flag = 0

		    acl_ptr_5 = new_sai_attribute_value_tp()
		    acl_ptr_5.booldata= True
		    for ix in range(0,len(nameList),1):
			    if "admin_state" == nameList[ix]:
				    dataList[ix] = int(dataList[ix])
				    acl_ptr_5.booldata= bool(dataList[ix])

		    acl_ptr_6 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_l4_src_port" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_6.enable = True
				    acl_ptr_6.mask.u16= int(valList[0])
				    acl_ptr_6.data.u16= int(valList[1])
		    if flag == 0:
			    acl_ptr_6.enable = False
		    flag = 0

		    acl_ptr_7 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_ip_protocol" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_7.enable = True
				    acl_ptr_7.mask.u8= int(valList[0])
				    acl_ptr_7.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_7.enable = False
		    flag = 0

		    acl_ptr_8 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_l4_dst_port" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_8.enable = True
				    acl_ptr_8.mask.u16= int(valList[0])
				    acl_ptr_8.data.u16= int(valList[1])
		    if flag == 0:
			    acl_ptr_8.enable = False
		    flag = 0

		    acl_ptr_9 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_dscp" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_9.enable = True
				    acl_ptr_9.mask.u8= int(valList[0])
				    acl_ptr_9.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_9.enable = False
		    flag = 0

		    acl_ptr_10 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_ipv6_next_header" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_10.enable = True
				    acl_ptr_10.mask.u8= int(valList[0])
				    acl_ptr_10.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_10.enable = False
		    flag = 0

		    acl_ptr_11 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_mirror_egress" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_11.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    acl_ptr_11.parameter.objlist.count = listLen
				    list_ptr = new_arr_sai_object(listLen)
				    for ix in range(0, listLen, 1):
					    arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
				    acl_ptr_11.parameter.objlist.list = list_ptr
		    if flag == 0:
			    acl_ptr_11.enable = False
		    flag = 0

		    acl_ptr_12 = new_sai_attribute_value_tp()
		    for ix in range(0,len(nameList),1):
			    if "priority" == nameList[ix]:
				    dataList[ix] = int(dataList[ix])
				    acl_ptr_12.u32= dataList[ix]

		    acl_ptr_13 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_dst_mac" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_13.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_13.mask.mac[ix] = int(postList[ix], 16)
				    postList = valList[1].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_13.data.mac[ix] = int(postList[ix], 16)
		    if flag == 0:
			    acl_ptr_13.enable = False
		    flag = 0

		    acl_ptr_14 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_in_port" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_14.enable = True
				    acl_ptr_14.data.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_14.enable = False
		    flag = 0

		    acl_ptr_15 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_acl_ip_type" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_15.enable = True
				    acl_ptr_15.mask.s32 = int(valList[0])
				    acl_ptr_15.data.s32 = int(valList[1])
		    if flag == 0:
			    acl_ptr_15.enable = False
		    flag = 0

		    acl_ptr_16 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_src_ip" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_16.enable = True
				    postList = valList[0].split(".")
				    postList = postList[::-1]
				    listLen = len(postList)
				    acl_ptr_16.mask.ip4 = 0
				    for ix in range(listLen):
					    acl_ptr_16.mask.ip4 = (acl_ptr_16.mask.ip4*256)+int(postList[ix])
				    postList = valList[1].split(".")
				    postList = postList[::-1]
				    listLen = len(postList)
				    acl_ptr_16.data.ip4 = 0
				    for ix in range(listLen):
					    acl_ptr_16.data.ip4 = (acl_ptr_16.data.ip4*256)+int(postList[ix])
		    if flag == 0:
			    acl_ptr_16.enable = False
		    flag = 0

		    acl_ptr_17 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_dst_ipv6" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_17.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_17.mask.ip6[ix] = int(postList[ix])
				    postList = valList[1].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_17.data.ip6[ix] = int(postList[ix])
		    if flag == 0:
			    acl_ptr_17.enable = False
		    flag = 0

		    acl_ptr_18 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_outer_vlan_id" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_18.enable = True
				    acl_ptr_18.mask.u16= int(valList[0])
				    acl_ptr_18.data.u16= int(valList[1])
		    if flag == 0:
			    acl_ptr_18.enable = False
		    flag = 0

		    acl_ptr_19 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_dst_ip" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_19.enable = True
				    postList = valList[0].split(".")
				    postList = postList[::-1]
				    listLen = len(postList)
				    acl_ptr_19.mask.ip4 = 0
				    for ix in range(listLen):
					    acl_ptr_19.mask.ip4 = (acl_ptr_19.mask.ip4*256)+int(postList[ix])
				    postList = valList[1].split(".")
				    postList = postList[::-1]
				    listLen = len(postList)
				    acl_ptr_19.data.ip4 = 0
				    for ix in range(listLen):
					    acl_ptr_19.data.ip4 = (acl_ptr_19.data.ip4*256)+int(postList[ix])
		    if flag == 0:
			    acl_ptr_19.enable = False
		    flag = 0

		    acl_ptr_20 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_counter" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_20.enable = True
				    acl_ptr_20.parameter.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_20.enable = False
		    flag = 0

		    acl_ptr_21 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_tc" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_21.enable = True
				    acl_ptr_21.mask.u8= int(valList[0])
				    acl_ptr_21.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_21.enable = False
		    flag = 0

		    acl_ptr_22 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_tos" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_22.enable = True
				    acl_ptr_22.mask.u8= int(valList[0])
				    acl_ptr_22.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_22.enable = False
		    flag = 0

		    acl_ptr_23 = new_sai_attribute_value_tp()
		    for ix in range(0,len(nameList),1):
			    if "table_id" == nameList[ix]:
				    dataList[ix] = int(dataList[ix])
				    acl_ptr_23.oid= dataList[ix]

		    acl_ptr_24 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_acl_range_type" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_24.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    acl_ptr_24.data.objlist.count = listLen
				    list_ptr = new_arr_sai_object(listLen)
				    for ix in range(0, listLen, 1):
					    arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
				    acl_ptr_24.data.objlist.list = list_ptr
		    if flag == 0:
			    acl_ptr_24.enable = False
		    flag = 0

		    acl_ptr_25 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_icmp_type" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_25.enable = True
				    acl_ptr_25.mask.u8= int(valList[0])
				    acl_ptr_25.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_25.enable = False
		    flag = 0

		    acl_ptr_26 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_src_ipv6" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_26.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_26.mask.ip6[ix] = int(postList[ix])
				    postList = valList[1].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_26.data.ip6[ix] = int(postList[ix])
		    if flag == 0:
			    acl_ptr_26.enable = False
		    flag = 0

		    acl_ptr_27 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_src_mac" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_27.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_27.mask.mac[ix] = int(postList[ix], 16)
				    postList = valList[1].split(".")
				    listLen = len(postList)
				    for ix in range(0, listLen, 1):
					    acl_ptr_27.data.mac[ix] = int(postList[ix], 16)
		    if flag == 0:
			    acl_ptr_27.enable = False
		    flag = 0

		    acl_ptr_28 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_tcp_flags" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_28.enable = True
				    acl_ptr_28.mask.u8= int(valList[0])
				    acl_ptr_28.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_28.enable = False
		    flag = 0

		    acl_ptr_29 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_icmp_code" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_29.enable = True
				    acl_ptr_29.mask.u8= int(valList[0])
				    acl_ptr_29.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_29.enable = False
		    flag = 0

		    acl_ptr_30 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_ether_type" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_30.enable = True
				    acl_ptr_30.mask.u16= int(valList[0])
				    acl_ptr_30.data.u16= int(valList[1])
		    if flag == 0:
			    acl_ptr_30.enable = False
		    flag = 0

		    acl_ptr_31 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_out_port" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_31.enable = True
				    acl_ptr_31.data.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_31.enable = False
		    flag = 0

		    acl_ptr_32 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_packet_action" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_32.enable = True
				    acl_ptr_32.parameter.s32 = int(valList[0])
		    if flag == 0:
			    acl_ptr_32.enable = False
		    flag = 0

		    acl_ptr_33 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_ingress_samplepacket_enable" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_33.enable = True
				    acl_ptr_33.parameter.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_33.enable = False
		    flag = 0

		    acl_ptr_34 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_icmpv6_type" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_34.enable = True
				    acl_ptr_34.mask.u8= int(valList[0])
				    acl_ptr_34.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_34.enable = False
		    flag = 0

		    acl_ptr_35 = new_shell_acl_action_tp()
		    for ix in range(0,len(nameList),1):
			    if "action_redirect" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_35.enable = True
				    acl_ptr_35.parameter.oid = int(valList[0])
		    if flag == 0:
			    acl_ptr_35.enable = False
		    flag = 0

		    acl_ptr_36 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_in_ports" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_36.enable = True
				    postList = valList[0].split(".")
				    listLen = len(postList)
				    acl_ptr_36.data.objlist.count = listLen
				    list_ptr = new_arr_sai_object(listLen)
				    for ix in range(0, listLen, 1):
					    arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
				    acl_ptr_36.data.objlist.list = list_ptr
		    if flag == 0:
			    acl_ptr_36.enable = False
		    flag = 0

		    acl_ptr_37 = new_shell_acl_field_tp()
		    for ix in range(0,len(nameList),1):
			    if "field_icmpv6_code" == nameList[ix]:
				    flag = 1
				    valList = dataList[ix].split(',')
				    for ix in range(0,len(valList),1):
					    valList[ix] = valList[ix].replace('[','').replace(']','').replace("'",'')
				    acl_ptr_37.enable = True
				    acl_ptr_37.mask.u8= int(valList[0])
				    acl_ptr_37.data.u8= int(valList[1])
		    if flag == 0:
			    acl_ptr_37.enable = False
		    flag = 0

		    ret = sai_create_acl_entry_custom(int(args[0]),acl_ptr_1,acl_ptr_2,acl_ptr_3,acl_ptr_4,acl_ptr_5.booldata,acl_ptr_6,acl_ptr_7,acl_ptr_8,acl_ptr_9,acl_ptr_10,acl_ptr_11,acl_ptr_12.u32,acl_ptr_13,acl_ptr_14,acl_ptr_15,acl_ptr_16,acl_ptr_17,acl_ptr_18,acl_ptr_19,acl_ptr_20,acl_ptr_21,acl_ptr_22,acl_ptr_23.oid,acl_ptr_24,acl_ptr_25,acl_ptr_26,acl_ptr_27,acl_ptr_28,acl_ptr_29,acl_ptr_30,acl_ptr_31,acl_ptr_32,acl_ptr_33,acl_ptr_34,acl_ptr_35,acl_ptr_36,acl_ptr_37)
		    delete_shell_acl_field_tp(acl_ptr_1)
		    delete_shell_acl_action_tp(acl_ptr_2)
		    delete_shell_acl_action_tp(acl_ptr_3)
		    delete_shell_acl_action_tp(acl_ptr_4)
		    delete_sai_attribute_value_tp(acl_ptr_5)
		    delete_shell_acl_field_tp(acl_ptr_6)
		    delete_shell_acl_field_tp(acl_ptr_7)
		    delete_shell_acl_field_tp(acl_ptr_8)
		    delete_shell_acl_field_tp(acl_ptr_9)
		    delete_shell_acl_field_tp(acl_ptr_10)
		    delete_shell_acl_action_tp(acl_ptr_11)
		    delete_sai_attribute_value_tp(acl_ptr_12)
		    delete_shell_acl_field_tp(acl_ptr_13)
		    delete_shell_acl_field_tp(acl_ptr_14)
		    delete_shell_acl_field_tp(acl_ptr_15)
		    delete_shell_acl_field_tp(acl_ptr_16)
		    delete_shell_acl_field_tp(acl_ptr_17)
		    delete_shell_acl_field_tp(acl_ptr_18)
		    delete_shell_acl_field_tp(acl_ptr_19)
		    delete_shell_acl_action_tp(acl_ptr_20)
		    delete_shell_acl_field_tp(acl_ptr_21)
		    delete_shell_acl_field_tp(acl_ptr_22)
		    delete_sai_attribute_value_tp(acl_ptr_23)
		    delete_shell_acl_field_tp(acl_ptr_24)
		    delete_shell_acl_field_tp(acl_ptr_25)
		    delete_shell_acl_field_tp(acl_ptr_26)
		    delete_shell_acl_field_tp(acl_ptr_27)
		    delete_shell_acl_field_tp(acl_ptr_28)
		    delete_shell_acl_field_tp(acl_ptr_29)
		    delete_shell_acl_field_tp(acl_ptr_30)
		    delete_shell_acl_field_tp(acl_ptr_31)
		    delete_shell_acl_action_tp(acl_ptr_32)
		    delete_shell_acl_action_tp(acl_ptr_33)
		    delete_shell_acl_field_tp(acl_ptr_34)
		    delete_shell_acl_action_tp(acl_ptr_35)
		    delete_shell_acl_field_tp(acl_ptr_36)
		    delete_shell_acl_field_tp(acl_ptr_37)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_acl_counter
    #/*********************************************************/
    def do_sai_create_acl_counter(self, arg):

        ''' sai_create_acl_counter '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, table_id, enable_byte_count, enable_packet_count'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','table_id','enable_byte_count','enable_packet_count' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_counter_default(int(args[0]),int(args[1]),int(args[2]) ,int(args[3]) )
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, table_id, packets, bytes, enable_byte_count, enable_packet_count'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','table_id','packets','bytes','enable_byte_count','enable_packet_count' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_acl_counter_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]),int(args[5]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_acl_table
    #/*********************************************************/
    def do_sai_create_acl_table(self, arg):

        ''' sai_create_acl_table '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, field_out_port, field_dst_ipv6, field_acl_range_type, field_tos, field_ether_type, acl_stage, field_acl_ip_type, acl_action_type_list, field_tcp_flags, field_in_port, field_dscp, field_src_mac, field_out_ports, field_in_ports, field_dst_ip, field_l4_dst_port, size, field_src_ipv6, field_dst_mac, field_tc, field_icmpv6_type, field_src_ip, field_ip_protocol, field_outer_vlan_id, field_icmpv6_code, field_ipv6_next_header, acl_bind_point_type_list, field_l4_src_port, field_icmp_type, field_icmp_code'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','field_out_port','field_dst_ipv6','field_acl_range_type','field_tos','field_ether_type','acl_stage','field_acl_ip_type','acl_action_type_list','field_tcp_flags','field_in_port','field_dscp','field_src_mac','field_out_ports','field_in_ports','field_dst_ip','field_l4_dst_port','size','field_src_ipv6','field_dst_mac','field_tc','field_icmpv6_type','field_src_ip','field_ip_protocol','field_outer_vlan_id','field_icmpv6_code','field_ipv6_next_header','acl_bind_point_type_list','field_l4_src_port','field_icmp_type','field_icmp_code' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[3] = args[3].replace(".",":").replace(",",":")
		    args[3] = args[3].strip(',').strip("'")
		    if args[3] != "[]":
			    postList = args[3].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    field_acl_range_type_ptr = new_sai_s32_list_tp()
		    field_acl_range_type_ptr.count = listLen
		    field_acl_range_type_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(field_acl_range_type_list_ptr, ix, int(postList[ix]))
		    field_acl_range_type_ptr.list = field_acl_range_type_list_ptr
		    listLen = 0
		    args[8] = args[8].replace(".",":").replace(",",":")
		    args[8] = args[8].strip(',').strip("'")
		    if args[8] != "[]":
			    postList = args[8].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    acl_action_type_list_ptr = new_sai_s32_list_tp()
		    acl_action_type_list_ptr.count = listLen
		    acl_action_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_action_type_list_list_ptr, ix, int(postList[ix]))
		    acl_action_type_list_ptr.list = acl_action_type_list_list_ptr
		    listLen = 0
		    args[27] = args[27].replace(".",":").replace(",",":")
		    args[27] = args[27].strip(',').strip("'")
		    if args[27] != "[]":
			    postList = args[27].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    acl_bind_point_type_list_ptr = new_sai_s32_list_tp()
		    acl_bind_point_type_list_ptr.count = listLen
		    acl_bind_point_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_bind_point_type_list_list_ptr, ix, int(postList[ix]))
		    acl_bind_point_type_list_ptr.list = acl_bind_point_type_list_list_ptr
		    ret = sai_create_acl_table_default(int(args[0]),int(args[1]) ,int(args[2]) ,field_acl_range_type_ptr,int(args[4]) ,int(args[5]) ,eval(args[6]),int(args[7]) ,acl_action_type_list_ptr,int(args[9]) ,int(args[10]) ,int(args[11]) ,int(args[12]) ,int(args[13]) ,int(args[14]) ,int(args[15]) ,int(args[16]) ,int(args[17]),int(args[18]) ,int(args[19]) ,int(args[20]) ,int(args[21]) ,int(args[22]) ,int(args[23]) ,int(args[24]) ,int(args[25]) ,int(args[26]) ,acl_bind_point_type_list_ptr,int(args[28]) ,int(args[29]) ,int(args[30]) )
		    delete_sai_s32_list_tp(field_acl_range_type_ptr)
		    delete_arr_sai_s32(field_acl_range_type_list_ptr)
		    delete_sai_s32_list_tp(acl_action_type_list_ptr)
		    delete_arr_sai_s32(acl_action_type_list_list_ptr)
		    delete_sai_s32_list_tp(acl_bind_point_type_list_ptr)
		    delete_arr_sai_s32(acl_bind_point_type_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, field_out_port, field_dst_ipv6, field_acl_range_type, field_tos, field_ether_type, acl_stage, field_acl_ip_type, acl_action_type_list, field_tcp_flags, field_in_port, field_dscp, field_src_mac, field_out_ports, field_in_ports, field_dst_ip, field_l4_dst_port, size, field_src_ipv6, field_dst_mac, field_tc, field_icmpv6_type, field_src_ip, field_ip_protocol, field_outer_vlan_id, field_icmpv6_code, field_ipv6_next_header, acl_bind_point_type_list, field_l4_src_port, field_icmp_type, field_icmp_code'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','field_out_port','field_dst_ipv6','field_acl_range_type','field_tos','field_ether_type','acl_stage','field_acl_ip_type','acl_action_type_list','field_tcp_flags','field_in_port','field_dscp','field_src_mac','field_out_ports','field_in_ports','field_dst_ip','field_l4_dst_port','size','field_src_ipv6','field_dst_mac','field_tc','field_icmpv6_type','field_src_ip','field_ip_protocol','field_outer_vlan_id','field_icmpv6_code','field_ipv6_next_header','acl_bind_point_type_list','field_l4_src_port','field_icmp_type','field_icmp_code' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[3] = args[3].replace(".",":").replace(",",":")
		    args[3] = args[3].strip(',').strip("'")
		    if args[3] != "[]":
			    postList = args[3].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    field_acl_range_type_ptr = new_sai_s32_list_tp()
		    field_acl_range_type_ptr.count = listLen
		    field_acl_range_type_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(field_acl_range_type_list_ptr, ix, int(postList[ix]))
		    field_acl_range_type_ptr.list = field_acl_range_type_list_ptr
		    listLen = 0
		    args[8] = args[8].replace(".",":").replace(",",":")
		    args[8] = args[8].strip(',').strip("'")
		    if args[8] != "[]":
			    postList = args[8].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    acl_action_type_list_ptr = new_sai_s32_list_tp()
		    acl_action_type_list_ptr.count = listLen
		    acl_action_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_action_type_list_list_ptr, ix, int(postList[ix]))
		    acl_action_type_list_ptr.list = acl_action_type_list_list_ptr
		    listLen = 0
		    args[27] = args[27].replace(".",":").replace(",",":")
		    args[27] = args[27].strip(',').strip("'")
		    if args[27] != "[]":
			    postList = args[27].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    acl_bind_point_type_list_ptr = new_sai_s32_list_tp()
		    acl_bind_point_type_list_ptr.count = listLen
		    acl_bind_point_type_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(acl_bind_point_type_list_list_ptr, ix, int(postList[ix]))
		    acl_bind_point_type_list_ptr.list = acl_bind_point_type_list_list_ptr
		    ret = sai_create_acl_table_custom(int(args[0]),int(args[1]),int(args[2]),field_acl_range_type_ptr,int(args[4]),int(args[5]),eval(args[6]),int(args[7]),acl_action_type_list_ptr,int(args[9]),int(args[10]),int(args[11]),int(args[12]),int(args[13]),int(args[14]),int(args[15]),int(args[16]),int(args[17]),int(args[18]),int(args[19]),int(args[20]),int(args[21]),int(args[22]),int(args[23]),int(args[24]),int(args[25]),int(args[26]),acl_bind_point_type_list_ptr,int(args[28]),int(args[29]),int(args[30]))
		    delete_sai_s32_list_tp(field_acl_range_type_ptr)
		    delete_arr_sai_s32(field_acl_range_type_list_ptr)
		    delete_sai_s32_list_tp(acl_action_type_list_ptr)
		    delete_arr_sai_s32(acl_action_type_list_list_ptr)
		    delete_sai_s32_list_tp(acl_bind_point_type_list_ptr)
		    delete_arr_sai_s32(acl_bind_point_type_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_acl_table_group_member_attribute
    #/*********************************************************/
    def do_sai_set_acl_table_group_member_attribute(self, arg):

	'''sai_set_acl_table_group_member_attribute: Enter[ acl_table_group_member_id, attr_id, attr_value ]'''

	varStr = 'acl_table_group_member_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_member_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_table_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_table_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_acl_table_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_acl_range_attribute
    #/*********************************************************/
    def do_sai_set_acl_range_attribute(self, arg):

	'''sai_set_acl_range_attribute: Enter[ acl_range_id, attr_id, attr_value ]'''

	varStr = 'acl_range_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_range_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ACL_RANGE_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_acl_range_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_RANGE_ATTR_LIMIT'):
			args[2]= int(args[2])
			attr_value_ptr.u32range= args[2]
			ret = sai_set_acl_range_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_acl_table_group_attribute
    #/*********************************************************/
    def do_sai_set_acl_table_group_attribute(self, arg):

	'''sai_set_acl_table_group_attribute: Enter[ acl_table_group_id, attr_id, attr_value ]'''

	varStr = 'acl_table_group_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_acl_table_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_acl_table_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_GROUP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_acl_table_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_GROUP_ATTR_MEMBER_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_acl_table_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_acl_entry_attribute
    #/*********************************************************/
    def do_sai_set_acl_entry_attribute(self, arg):

	'''sai_set_acl_entry_attribute: Enter[ acl_entry_id, attr_id, attr_value ]'''

	varStr = 'acl_entry_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_entry_id','attr_id','attr_value' ]
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
		acl_field_ptr = new_shell_acl_field_tp()
		acl_action_ptr = new_shell_acl_action_tp()
		aclfieldflag = 0
		aclactionflag = 0
		if args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.data.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_field_ptr.data.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_ENDPOINT_IP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.ipaddr= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_action_ptr.parameter.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_action_ptr.parameter.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_GRE_KEY'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INT_INSERT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPV6'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL0_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ADMIN_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TUNNEL_VNI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL0_BOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DSCP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_ADD_VLAN_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u16= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_ADD_VLAN_PRI'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.mask.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.mask.ip4 = (acl_field_ptr.mask.ip4*256)+int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.data.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.data.ip4 = (acl_field_ptr.data.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_SRC_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_action_ptr.parameter.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_action_ptr.parameter.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.mac[ix] = int(postList[ix], 16)
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.mac[ix] = int(postList[ix], 16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_action_ptr.parameter.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL4_EXP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.s32 = int(args[3])
			acl_field_ptr.data.s32 = int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL2_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL2_BOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL0_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL2_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_ECMP_HASH_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_HAS_VLAN_TAG'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_DST_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_DO_NOT_LEARN'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u32= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_action_ptr.parameter.ip4 = 0
			for ix in range(listLen): 
				acl_action_ptr.parameter.ip4 = (acl_action_ptr.parameter.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6_WORD1'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_ISOLATION_GROUP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.s32 = int(args[3])
			acl_field_ptr.data.s32 = int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6_WORD0'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TAM_INT_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.s32 = int(args[3])
			acl_field_ptr.data.s32 = int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.mask.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.mask.ip4 = (acl_field_ptr.mask.ip4*256)+int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.data.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.data.ip4 = (acl_field_ptr.data.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IP_IDENTIFICATION'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DTEL_INT_SESSION'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_action_ptr.parameter.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6_WORD3'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6_WORD2'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL2_EXP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL3_EXP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6_WORD0'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_ACL_DTEL_FLOW_OP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.s32 = eval(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL0_EXP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u32= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.mask.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.mask.ip4 = (acl_field_ptr.mask.ip4*256)+int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.data.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.data.ip4 = (acl_field_ptr.data.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_action_ptr.parameter.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_action_ptr.parameter.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u16= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_action_ptr.parameter.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_action_ptr.parameter.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INT_REPORT_DROPS'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_AETH_SYNDROME'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_COUNTER'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.mask.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.mask.ip4 = (acl_field_ptr.mask.ip4*256)+int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_field_ptr.data.ip4 = 0
			for ix in range(listLen): 
				acl_field_ptr.data.ip4 = (acl_field_ptr.data.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_LAG_HASH_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_BTH_OPCODE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL3_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6_WORD2'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6_WORD3'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DTEL_DROP_REPORT_ENABLE'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6_WORD1'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TC'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL4_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MACSEC_SCI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u64= int(args[3])
			acl_field_ptr.data.u64= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.s32 = int(args[3])
			acl_field_ptr.data.s32 = int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_TABLE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_MACSEC_FLOW'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.s32 = eval(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.data.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_field_ptr.data.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_action_ptr.parameter.mac[ix] = int(postList[ix], 16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL1_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL4_BOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INT_REPORT_FLOW'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_NO_NAT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.mac[ix] = int(postList[ix], 16)
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.mac[ix] = int(postList[ix], 16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_TC'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DTEL_TAIL_DROP_REPORT_ENABLE'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u16= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL3_LABEL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u16= int(args[3])
			acl_field_ptr.data.u16= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INT_DELETE'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.s32 = eval(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_FLOOD'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.s32= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INT_REPORT_TAIL_DROPS'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL3_BOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MAX'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.mask.u8list.count = listLen
			list_ptr1 = new_arr_sai_u8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u8_setitem(list_ptr1, ix, int(postList[ix]))
			acl_field_ptr.mask.u8list.list = list_ptr1
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.data.u8list.count = listLen
			list_ptr2 = new_arr_sai_u8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u8_setitem(list_ptr2, ix, int(postList[ix]))
			acl_field_ptr.data.u8list.list = list_ptr2
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_u8(list_ptr2)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL1_BOS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.data.u8 = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MIN'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.mask.u8list.count = listLen
			list_ptr1 = new_arr_sai_u8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u8_setitem(list_ptr1, ix, int(postList[ix]))
			acl_field_ptr.mask.u8list.list = list_ptr1
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.data.u8list.count = listLen
			list_ptr2 = new_arr_sai_u8(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u8_setitem(list_ptr2, ix, int(postList[ix]))
			acl_field_ptr.data.u8list.list = list_ptr2
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_u8(list_ptr2)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_action_ptr.parameter.mac[ix] = int(postList[ix], 16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_TYPE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL1_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_TAM_INT_OBJECT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DTEL_FLOW_SAMPLE_PERCENT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u8= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u16= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.oid = int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL1_EXP'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u32= int(args[3])
			acl_field_ptr.data.u32= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.booldata= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			acl_action_ptr.parameter.ip4 = 0
			for ix in range(listLen): 
				acl_action_ptr.parameter.ip4 = (acl_action_ptr.parameter.ip4*256)+int(postList[ix])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			acl_field_ptr.data.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			acl_field_ptr.data.objlist.list = list_ptr
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_MPLS_LABEL4_TTL'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_CODE'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPV6'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.mask.ip6[ix] = int(postList[ix])
			args[4] = args[4].replace(".",":").replace(",",":")
			postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				acl_field_ptr.data.ip6[ix] = int(postList[ix],16)
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID'):
			aclactionflag = 1 
			acl_action_ptr.enable = bool(int(args[2]))
			acl_action_ptr.parameter.u32= int(args[3])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		elif args[1] == eval('SAI_ACL_ENTRY_ATTR_FIELD_ECN'):
			aclfieldflag = 1 
			acl_field_ptr.enable = bool(int(args[2]))
			acl_field_ptr.mask.u8= int(args[3])
			acl_field_ptr.data.u8= int(args[4])
			ret = sai_set_acl_entry_attribute(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
		delete_shell_acl_field_tp(acl_field_ptr)
		delete_shell_acl_action_tp(acl_action_ptr)
    #/*********************************************************/
    # command for sai_set_acl_counter_attribute
    #/*********************************************************/
    def do_sai_set_acl_counter_attribute(self, arg):

	'''sai_set_acl_counter_attribute: Enter[ acl_counter_id, attr_id, attr_value ]'''

	varStr = 'acl_counter_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_counter_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ACL_COUNTER_ATTR_TABLE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_COUNTER_ATTR_PACKETS'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_acl_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_COUNTER_ATTR_BYTES'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_acl_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_acl_table_attribute
    #/*********************************************************/
    def do_sai_set_acl_table_attribute(self, arg):

	'''sai_set_acl_table_attribute: Enter[ acl_table_id, attr_id, attr_value ]'''

	varStr = 'acl_table_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL4_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL4_EXP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL4_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_ETHER_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_ACL_STAGE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_ENTRY_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MACSEC_SCI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL1_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL3_BOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6_WORD0'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IN_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL0_BOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TAM_INT_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL3_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DSCP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL1_EXP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL0_EXP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ECN'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL3_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MAX'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL2_BOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_L4_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL0_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TUNNEL_VNI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_GRE_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL3_EXP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6_WORD3'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6_WORD2'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6_WORD1'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_AETH_SYNDROME'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL0_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_HAS_VLAN_TAG'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_IP_PROTOCOL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6_WORD3'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6_WORD0'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6_WORD1'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_MAC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL2_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_SRC_IP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL2_EXP'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IP_IDENTIFICATION'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL1_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_CODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_INNER_L4_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL2_LABEL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_IPV6_NEXT_HEADER'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL1_BOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_MPLS_LABEL4_BOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_BTH_OPCODE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6_WORD2'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_acl_table_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_acl_table_group_member_attribute
    #/*********************************************************/
    def do_sai_get_acl_table_group_member_attribute(self, arg):

	'''sai_get_acl_table_group_member_attribute: Enter[  acl_table_group_member_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_table_group_member_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_member_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_table_group_member_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_acl_range_attribute
    #/*********************************************************/
    def do_sai_get_acl_range_attribute(self, arg):

	'''sai_get_acl_range_attribute: Enter[  acl_range_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_range_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_range_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_range_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_acl_table_group_attribute
    #/*********************************************************/
    def do_sai_get_acl_table_group_attribute(self, arg):

	'''sai_get_acl_table_group_attribute: Enter[  acl_table_group_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_table_group_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_table_group_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_acl_entry_attribute
    #/*********************************************************/
    def do_sai_get_acl_entry_attribute(self, arg):

	'''sai_get_acl_entry_attribute: Enter[  acl_entry_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_entry_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_entry_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_entry_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_acl_counter_attribute
    #/*********************************************************/
    def do_sai_get_acl_counter_attribute(self, arg):

	'''sai_get_acl_counter_attribute: Enter[  acl_counter_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_counter_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_counter_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_counter_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_acl_table_attribute
    #/*********************************************************/
    def do_sai_get_acl_table_attribute(self, arg):

	'''sai_get_acl_table_attribute: Enter[  acl_table_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' acl_table_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_acl_table_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_table_group_member
    #/*********************************************************/
    def do_sai_remove_acl_table_group_member(self, arg):

	'''sai_remove_acl_table_group_member: Enter[  acl_table_group_member_id ]'''

	varStr = ' acl_table_group_member_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_member_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_table_group_member(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_range
    #/*********************************************************/
    def do_sai_remove_acl_range(self, arg):

	'''sai_remove_acl_range: Enter[  acl_range_id ]'''

	varStr = ' acl_range_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_range_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_range(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_table_group
    #/*********************************************************/
    def do_sai_remove_acl_table_group(self, arg):

	'''sai_remove_acl_table_group: Enter[  acl_table_group_id ]'''

	varStr = ' acl_table_group_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_group_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_table_group(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_entry
    #/*********************************************************/
    def do_sai_remove_acl_entry(self, arg):

	'''sai_remove_acl_entry: Enter[  acl_entry_id ]'''

	varStr = ' acl_entry_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_entry_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_entry(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_counter
    #/*********************************************************/
    def do_sai_remove_acl_counter(self, arg):

	'''sai_remove_acl_counter: Enter[  acl_counter_id ]'''

	varStr = ' acl_counter_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_counter_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_counter(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_acl_table
    #/*********************************************************/
    def do_sai_remove_acl_table(self, arg):

	'''sai_remove_acl_table: Enter[  acl_table_id ]'''

	varStr = ' acl_table_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'acl_table_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_acl_table(int(args[0]))
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
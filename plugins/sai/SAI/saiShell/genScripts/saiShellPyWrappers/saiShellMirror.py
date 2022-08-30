#!/usr/bin/env python
#  saiShellMirror.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellMirror.py
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
# The class object for saiShellMirror operations
#/**********************************************************************************/

class saiShellMirrorObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_mirror_session
    #/*********************************************************/
    def do_sai_create_mirror_session(self, arg):

        ''' sai_create_mirror_session '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, ip_addr_family, src_ip_address, ip_addr_family, dst_ip_address, monitor_portlist_valid, erspan_encapsulation_type, type, monitor_port, dst_mac_address, src_mac_address, gre_protocol_type, iphdr_version, tos'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ip_addr_family','src_ip_address','ip_addr_family','dst_ip_address','monitor_portlist_valid','erspan_encapsulation_type','type','monitor_port','dst_mac_address','src_mac_address','gre_protocol_type','iphdr_version','tos' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[2] = args[2].replace(".",":").replace(",",":")
		    postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_ip_address_ptr = new_sai_ip_address_tp()
		    src_ip_address_ptr.addr_family = eval(args[1])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_address_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_address_ptr = new_sai_ip_address_tp()
		    dst_ip_address_ptr.addr_family = eval(args[3])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_address_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[9] = args[9].replace(".",":").replace(",",":")
		    postList = args[9].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    dst_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    args[10] = args[10].replace(".",":").replace(",",":")
		    postList = args[10].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    src_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_mirror_session_default(int(args[0]),src_ip_address_ptr,dst_ip_address_ptr,int(args[5]) ,eval(args[6]),eval(args[7]),int(args[8]),dst_mac_address_ptr,src_mac_address_ptr,int(args[11]),int(args[12]),int(args[13]))
		    delete_sai_ip_address_tp(src_ip_address_ptr)
		    delete_sai_ip_address_tp(dst_ip_address_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, ip_addr_family, src_ip_address, vlan_pri, ip_addr_family, dst_ip_address, monitor_portlist_valid, truncate_size, erspan_encapsulation_type, vlan_header_valid, vlan_tpid, type, vlan_cfi, monitor_port, sample_rate, ttl, dst_mac_address, src_mac_address, gre_protocol_type, tc, vlan_id, iphdr_version, tos, policer'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ip_addr_family','src_ip_address','vlan_pri','ip_addr_family','dst_ip_address','monitor_portlist_valid','truncate_size','erspan_encapsulation_type','vlan_header_valid','vlan_tpid','type','vlan_cfi','monitor_port','sample_rate','ttl','dst_mac_address','src_mac_address','gre_protocol_type','tc','vlan_id','iphdr_version','tos','policer' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[2] = args[2].replace(".",":").replace(",",":")
		    postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_ip_address_ptr = new_sai_ip_address_tp()
		    src_ip_address_ptr.addr_family = eval(args[1])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_address_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_address_ptr = new_sai_ip_address_tp()
		    dst_ip_address_ptr.addr_family = eval(args[4])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_address_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[16] = args[16].replace(".",":").replace(",",":")
		    postList = args[16].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    dst_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    args[17] = args[17].replace(".",":").replace(",",":")
		    postList = args[17].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    src_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_mirror_session_custom(int(args[0]),src_ip_address_ptr,int(args[3]),dst_ip_address_ptr,int(args[6]),int(args[7]),eval(args[8]),int(args[9]),int(args[10]),eval(args[11]),int(args[12]),int(args[13]),int(args[14]),int(args[15]),dst_mac_address_ptr,src_mac_address_ptr,int(args[18]),int(args[19]),int(args[20]),int(args[21]),int(args[22]),int(args[23]))
		    delete_sai_ip_address_tp(src_ip_address_ptr)
		    delete_sai_ip_address_tp(dst_ip_address_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_mirror_session_attribute
    #/*********************************************************/
    def do_sai_set_mirror_session_attribute(self, arg):

	'''sai_set_mirror_session_attribute: Enter[ mirror_session_id, attr_id, attr_value ]'''

	varStr = 'mirror_session_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'mirror_session_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS'):
			ipaddrflag = 1
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			ipaddr_ptr.addr_family = eval(args[2])
			base = 10
			if listLen > 4:
				base = 16
			for ix in range(0, listLen, 1):
				ipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_VLAN_PRI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS'):
			ipaddrflag = 1
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			ipaddr_ptr.addr_family = eval(args[2])
			base = 10
			if listLen > 4:
				base = 16
			for ix in range(0, listLen, 1):
				ipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_MONITOR_PORTLIST_VALID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_UDP_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_CONGESTION_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_VLAN_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_VLAN_CFI'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_MONITOR_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_MONITOR_PORTLIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_TC'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_TOS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_UDP_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MIRROR_SESSION_ATTR_POLICER'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_mirror_session_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_mirror_session_attribute
    #/*********************************************************/
    def do_sai_get_mirror_session_attribute(self, arg):

	'''sai_get_mirror_session_attribute: Enter[  mirror_session_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' mirror_session_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'mirror_session_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_mirror_session_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_mirror_session
    #/*********************************************************/
    def do_sai_remove_mirror_session(self, arg):

	'''sai_remove_mirror_session: Enter[  mirror_session_id ]'''

	varStr = ' mirror_session_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'mirror_session_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_mirror_session(int(args[0]))
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
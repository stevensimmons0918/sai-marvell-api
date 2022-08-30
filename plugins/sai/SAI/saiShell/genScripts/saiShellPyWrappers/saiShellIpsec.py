#!/usr/bin/env python
#  saiShellIpsec.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellIpsec.py
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
# The class object for saiShellIpsec operations
#/**********************************************************************************/

class saiShellIpsecObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_ipsec_sa
    #/*********************************************************/
    def do_sai_create_ipsec_sa(self, arg):

        ''' sai_create_ipsec_sa '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, term_vlan_id_enable, auth_key, term_src_ip_enable, encrypt_key, ipsec_spi, term_vlan_id, ipsec_direction, ipsec_cipher, ipsec_esn_enable, ipsec_id, salt, ip_addr_family, term_src_ip, ip_addr_family, term_dst_ip'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','term_vlan_id_enable','auth_key','term_src_ip_enable','encrypt_key','ipsec_spi','term_vlan_id','ipsec_direction','ipsec_cipher','ipsec_esn_enable','ipsec_id','salt','ip_addr_family','term_src_ip','ip_addr_family','term_dst_ip' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[13] = args[13].replace(".",":").replace(",",":")
		    postList = args[13].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    term_src_ip_ptr = new_sai_ip_address_tp()
		    term_src_ip_ptr.addr_family = eval(args[12])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    term_src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[15] = args[15].replace(".",":").replace(",",":")
		    postList = args[15].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    term_dst_ip_ptr = new_sai_ip_address_tp()
		    term_dst_ip_ptr.addr_family = eval(args[14])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    term_dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_ipsec_sa_default(int(args[0]),int(args[1]) ,int(args[2]),int(args[3]) ,int(args[4]),int(args[5]),int(args[6]),eval(args[7]),eval(args[8]),int(args[9]) ,int(args[10]),int(args[11]),term_src_ip_ptr,term_dst_ip_ptr)
		    delete_sai_ip_address_tp(term_src_ip_ptr)
		    delete_sai_ip_address_tp(term_dst_ip_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, term_vlan_id_enable, auth_key, ipsec_port_list, egress_esn, external_sa_index, term_src_ip_enable, ipsec_replay_protection_window, ipsec_replay_protection_enable, minimum_ingress_esn, encrypt_key, ipsec_spi, term_vlan_id, ipsec_direction, ipsec_cipher, ipsec_esn_enable, ipsec_id, salt, ip_addr_family, term_src_ip, ip_addr_family, term_dst_ip'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','term_vlan_id_enable','auth_key','ipsec_port_list','egress_esn','external_sa_index','term_src_ip_enable','ipsec_replay_protection_window','ipsec_replay_protection_enable','minimum_ingress_esn','encrypt_key','ipsec_spi','term_vlan_id','ipsec_direction','ipsec_cipher','ipsec_esn_enable','ipsec_id','salt','ip_addr_family','term_src_ip','ip_addr_family','term_dst_ip' ]
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
		    ipsec_port_list_ptr = new_sai_object_list_tp()
		    ipsec_port_list_ptr.count = listLen
		    ipsec_port_list_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(ipsec_port_list_list_ptr, ix, int(postList[ix]))
		    ipsec_port_list_ptr.list = ipsec_port_list_list_ptr
		    args[19] = args[19].replace(".",":").replace(",",":")
		    postList = args[19].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    term_src_ip_ptr = new_sai_ip_address_tp()
		    term_src_ip_ptr.addr_family = eval(args[18])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    term_src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[21] = args[21].replace(".",":").replace(",",":")
		    postList = args[21].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    term_dst_ip_ptr = new_sai_ip_address_tp()
		    term_dst_ip_ptr.addr_family = eval(args[20])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    term_dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_ipsec_sa_custom(int(args[0]),int(args[1]),int(args[2]),ipsec_port_list_ptr,int(args[4]),int(args[5]),int(args[6]),int(args[7]),int(args[8]),int(args[9]),int(args[10]),int(args[11]),int(args[12]),eval(args[13]),eval(args[14]),int(args[15]),int(args[16]),int(args[17]),term_src_ip_ptr,term_dst_ip_ptr)
		    delete_sai_object_list_tp(ipsec_port_list_ptr)
		    delete_arr_sai_object(ipsec_port_list_list_ptr)
		    delete_sai_ip_address_tp(term_src_ip_ptr)
		    delete_sai_ip_address_tp(term_dst_ip_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_ipsec_port
    #/*********************************************************/
    def do_sai_create_ipsec_port(self, arg):

        ''' sai_create_ipsec_port '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, native_vlan_id, port_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','native_vlan_id','port_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ipsec_port_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, ctag_enable, vrf_from_packet_vlan_enable, switch_switching_mode, native_vlan_id, stag_enable, port_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ctag_enable','vrf_from_packet_vlan_enable','switch_switching_mode','native_vlan_id','stag_enable','port_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ipsec_port_custom(int(args[0]),int(args[1]),int(args[2]),eval(args[3]),int(args[4]),int(args[5]),int(args[6]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_ipsec
    #/*********************************************************/
    def do_sai_create_ipsec(self, arg):

        ''' sai_create_ipsec '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, external_sa_index_enable'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','external_sa_index_enable' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ipsec_default(int(args[0]),int(args[1]) )
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, max_vlan_tags_parsed, octet_count_low_watermark, octet_count_high_watermark, ctag_tpid, stats_mode, warm_boot_enable, external_sa_index_enable, stag_tpid'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','max_vlan_tags_parsed','octet_count_low_watermark','octet_count_high_watermark','ctag_tpid','stats_mode','warm_boot_enable','external_sa_index_enable','stag_tpid' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_ipsec_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]),eval(args[5]),int(args[6]),int(args[7]),int(args[8]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_ipsec_sa_attribute
    #/*********************************************************/
    def do_sai_set_ipsec_sa_attribute(self, arg):

	'''sai_set_ipsec_sa_attribute: Enter[ ipsec_sa_id, attr_id, attr_value ]'''

	varStr = 'ipsec_sa_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_IPSEC_SA_ATTR_OCTET_COUNT_STATUS'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_TERM_VLAN_ID_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_SA_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_AUTH_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.authkey= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_EGRESS_ESN'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_EXTERNAL_SA_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_TERM_SRC_IP_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_REPLAY_PROTECTION_WINDOW'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_REPLAY_PROTECTION_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_MINIMUM_INGRESS_ESN'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_ENCRYPT_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.encrypt_key= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_SPI'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_TERM_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_DIRECTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_CIPHER'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_ESN_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_IPSEC_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_SALT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_TERM_SRC_IP'):
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
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_SA_ATTR_TERM_DST_IP'):
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
			ret = sai_set_ipsec_sa_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_ipsec_port_attribute
    #/*********************************************************/
    def do_sai_set_ipsec_port_attribute(self, arg):

	'''sai_set_ipsec_port_attribute: Enter[ ipsec_port_id, attr_id, attr_value ]'''

	varStr = 'ipsec_port_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_IPSEC_PORT_ATTR_CTAG_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_PORT_ATTR_VRF_FROM_PACKET_VLAN_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_PORT_ATTR_SWITCH_SWITCHING_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_PORT_ATTR_NATIVE_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_PORT_ATTR_STAG_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_PORT_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_ipsec_port_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_ipsec_attribute
    #/*********************************************************/
    def do_sai_set_ipsec_attribute(self, arg):

	'''sai_set_ipsec_attribute: Enter[ ipsec_id, attr_id, attr_value ]'''

	varStr = 'ipsec_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_IPSEC_ATTR_STATS_MODE_READ_CLEAR_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_MAX_VLAN_TAGS_PARSED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_AVAILABLE_IPSEC_SA'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_OCTET_COUNT_LOW_WATERMARK'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_TERM_REMOTE_IP_MATCH_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SWITCHING_MODE_CUT_THROUGH_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_ESN_64BIT_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_OCTET_COUNT_HIGH_WATERMARK'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_CTAG_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SYSTEM_SIDE_MTU'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SUPPORTED_CIPHER_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SWITCHING_MODE_STORE_AND_FORWARD_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_STATS_MODE_READ_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_STATS_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SA_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_WARM_BOOT_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_SN_32BIT_SUPPORTED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_WARM_BOOT_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_EXTERNAL_SA_INDEX_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_IPSEC_ATTR_STAG_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_ipsec_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_ipsec_sa_attribute
    #/*********************************************************/
    def do_sai_get_ipsec_sa_attribute(self, arg):

	'''sai_get_ipsec_sa_attribute: Enter[  ipsec_sa_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' ipsec_sa_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_ipsec_sa_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_ipsec_port_attribute
    #/*********************************************************/
    def do_sai_get_ipsec_port_attribute(self, arg):

	'''sai_get_ipsec_port_attribute: Enter[  ipsec_port_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' ipsec_port_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_ipsec_port_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_ipsec_attribute
    #/*********************************************************/
    def do_sai_get_ipsec_attribute(self, arg):

	'''sai_get_ipsec_attribute: Enter[  ipsec_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' ipsec_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_ipsec_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_ipsec_sa_stats
    #/*********************************************************/
    def do_sai_get_ipsec_sa_stats(self, arg):

	'''sai_get_ipsec_sa_stats: Enter[  ipsec_sa_id, number_of_counters ]'''

	varStr = ' ipsec_sa_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id','number_of_counters' ]
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
		ret = sai_get_ipsec_sa_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_ipsec_sa_stats
    #/*********************************************************/
    def do_sai_clear_ipsec_sa_stats(self, arg):

	'''sai_clear_ipsec_sa_stats: Enter[  ipsec_sa_id, number_of_counters ]'''

	varStr = ' ipsec_sa_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id','number_of_counters' ]
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
		ret = sai_clear_ipsec_sa_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_ipsec_port_stats
    #/*********************************************************/
    def do_sai_get_ipsec_port_stats(self, arg):

	'''sai_get_ipsec_port_stats: Enter[  ipsec_port_id, number_of_counters ]'''

	varStr = ' ipsec_port_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id','number_of_counters' ]
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
		ret = sai_get_ipsec_port_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_ipsec_port_stats
    #/*********************************************************/
    def do_sai_clear_ipsec_port_stats(self, arg):

	'''sai_clear_ipsec_port_stats: Enter[  ipsec_port_id, number_of_counters ]'''

	varStr = ' ipsec_port_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id','number_of_counters' ]
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
		ret = sai_clear_ipsec_port_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_ipsec_sa_stats_ext
    #/*********************************************************/
    def do_sai_get_ipsec_sa_stats_ext(self, arg):

	'''sai_get_ipsec_sa_stats_ext: Enter[  ipsec_sa_id, number_of_counters, mode ]'''

	varStr = ' ipsec_sa_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id','number_of_counters','mode' ]
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
		ret = sai_get_ipsec_sa_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_get_ipsec_port_stats_ext
    #/*********************************************************/
    def do_sai_get_ipsec_port_stats_ext(self, arg):

	'''sai_get_ipsec_port_stats_ext: Enter[  ipsec_port_id, number_of_counters, mode ]'''

	varStr = ' ipsec_port_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id','number_of_counters','mode' ]
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
		ret = sai_get_ipsec_port_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_ipsec_sa
    #/*********************************************************/
    def do_sai_remove_ipsec_sa(self, arg):

	'''sai_remove_ipsec_sa: Enter[  ipsec_sa_id ]'''

	varStr = ' ipsec_sa_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_sa_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_ipsec_sa(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_ipsec_port
    #/*********************************************************/
    def do_sai_remove_ipsec_port(self, arg):

	'''sai_remove_ipsec_port: Enter[  ipsec_port_id ]'''

	varStr = ' ipsec_port_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_port_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_ipsec_port(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_ipsec
    #/*********************************************************/
    def do_sai_remove_ipsec(self, arg):

	'''sai_remove_ipsec: Enter[  ipsec_id ]'''

	varStr = ' ipsec_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'ipsec_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_ipsec(int(args[0]))
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
#!/usr/bin/env python
#  saiShellTunnel.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellTunnel.py
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
# The class object for saiShellTunnel operations
#/**********************************************************************************/

class saiShellTunnelObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_tunnel
    #/*********************************************************/
    def do_sai_create_tunnel(self, arg):

        ''' sai_create_tunnel '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, underlay_interface, peer_mode, decap_mappers, ip_addr_family, encap_dst_ip, ip_addr_family, encap_src_ip, encap_mappers, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','underlay_interface','peer_mode','decap_mappers','ip_addr_family','encap_dst_ip','ip_addr_family','encap_src_ip','encap_mappers','type' ]
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
		    decap_mappers_ptr = new_sai_object_list_tp()
		    decap_mappers_ptr.count = listLen
		    decap_mappers_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(decap_mappers_list_ptr, ix, int(postList[ix]))
		    decap_mappers_ptr.list = decap_mappers_list_ptr
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    encap_dst_ip_ptr = new_sai_ip_address_tp()
		    encap_dst_ip_ptr.addr_family = eval(args[4])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    encap_dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[7] = args[7].replace(".",":").replace(",",":")
		    postList = args[7].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    encap_src_ip_ptr = new_sai_ip_address_tp()
		    encap_src_ip_ptr.addr_family = eval(args[6])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    encap_src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    listLen = 0
		    args[8] = args[8].replace(".",":").replace(",",":")
		    args[8] = args[8].strip(',').strip("'")
		    if args[8] != "[]":
			    postList = args[8].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    encap_mappers_ptr = new_sai_object_list_tp()
		    encap_mappers_ptr.count = listLen
		    encap_mappers_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(encap_mappers_list_ptr, ix, int(postList[ix]))
		    encap_mappers_ptr.list = encap_mappers_list_ptr
		    ret = sai_create_tunnel_default(int(args[0]),int(args[1]),eval(args[2]),decap_mappers_ptr,encap_dst_ip_ptr,encap_src_ip_ptr,encap_mappers_ptr,eval(args[9]))
		    delete_sai_object_list_tp(decap_mappers_ptr)
		    delete_arr_sai_object(decap_mappers_list_ptr)
		    delete_sai_ip_address_tp(encap_dst_ip_ptr)
		    delete_sai_ip_address_tp(encap_src_ip_ptr)
		    delete_sai_object_list_tp(encap_mappers_ptr)
		    delete_arr_sai_object(encap_mappers_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, underlay_interface, encap_dscp_mode, peer_mode, encap_ttl_val, decap_mappers, encap_dscp_val, ip_addr_family, encap_dst_ip, ip_addr_family, encap_src_ip, decap_ttl_mode, encap_mappers, encap_ttl_mode, type, decap_dscp_mode'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','underlay_interface','encap_dscp_mode','peer_mode','encap_ttl_val','decap_mappers','encap_dscp_val','ip_addr_family','encap_dst_ip','ip_addr_family','encap_src_ip','decap_ttl_mode','encap_mappers','encap_ttl_mode','type','decap_dscp_mode' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[5] = args[5].replace(".",":").replace(",",":")
		    args[5] = args[5].strip(',').strip("'")
		    if args[5] != "[]":
			    postList = args[5].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    decap_mappers_ptr = new_sai_object_list_tp()
		    decap_mappers_ptr.count = listLen
		    decap_mappers_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(decap_mappers_list_ptr, ix, int(postList[ix]))
		    decap_mappers_ptr.list = decap_mappers_list_ptr
		    args[8] = args[8].replace(".",":").replace(",",":")
		    postList = args[8].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    encap_dst_ip_ptr = new_sai_ip_address_tp()
		    encap_dst_ip_ptr.addr_family = eval(args[7])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    encap_dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[10] = args[10].replace(".",":").replace(",",":")
		    postList = args[10].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    encap_src_ip_ptr = new_sai_ip_address_tp()
		    encap_src_ip_ptr.addr_family = eval(args[9])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    encap_src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    listLen = 0
		    args[12] = args[12].replace(".",":").replace(",",":")
		    args[12] = args[12].strip(',').strip("'")
		    if args[12] != "[]":
			    postList = args[12].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    encap_mappers_ptr = new_sai_object_list_tp()
		    encap_mappers_ptr.count = listLen
		    encap_mappers_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(encap_mappers_list_ptr, ix, int(postList[ix]))
		    encap_mappers_ptr.list = encap_mappers_list_ptr
		    ret = sai_create_tunnel_custom(int(args[0]),int(args[1]),eval(args[2]),eval(args[3]),int(args[4]),decap_mappers_ptr,int(args[6]),encap_dst_ip_ptr,encap_src_ip_ptr,eval(args[11]),encap_mappers_ptr,eval(args[13]),eval(args[14]),eval(args[15]))
		    delete_sai_object_list_tp(decap_mappers_ptr)
		    delete_arr_sai_object(decap_mappers_list_ptr)
		    delete_sai_ip_address_tp(encap_dst_ip_ptr)
		    delete_sai_ip_address_tp(encap_src_ip_ptr)
		    delete_sai_object_list_tp(encap_mappers_ptr)
		    delete_arr_sai_object(encap_mappers_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tunnel_map_entry
    #/*********************************************************/
    def do_sai_create_tunnel_map_entry(self, arg):

        ''' sai_create_tunnel_map_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, tunnel_map_type, virtual_router_id_value, tunnel_map, vni_id_value, vlan_id_value, vlan_id_key, vni_id_key, virtual_router_id_key'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','tunnel_map_type','virtual_router_id_value','tunnel_map','vni_id_value','vlan_id_value','vlan_id_key','vni_id_key','virtual_router_id_key' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_tunnel_map_entry_default(int(args[0]),eval(args[1]),int(args[2]),int(args[3]),int(args[4]),int(args[5]),int(args[6]),int(args[7]),int(args[8]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, tunnel_map_type, virtual_router_id_value, tunnel_map, vni_id_value, vlan_id_value, vlan_id_key, vni_id_key, virtual_router_id_key'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','tunnel_map_type','virtual_router_id_value','tunnel_map','vni_id_value','vlan_id_value','vlan_id_key','vni_id_key','virtual_router_id_key' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_tunnel_map_entry_custom(int(args[0]),eval(args[1]),int(args[2]),int(args[3]),int(args[4]),int(args[5]),int(args[6]),int(args[7]),int(args[8]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tunnel_term_table_entry
    #/*********************************************************/
    def do_sai_create_tunnel_term_table_entry(self, arg):

        ''' sai_create_tunnel_term_table_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, ip_addr_family, src_ip, ip_addr_family, dst_ip_mask, action_tunnel_id, tunnel_type, ip_addr_family, dst_ip, type, vr_id, ip_addr_family, src_ip_mask'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ip_addr_family','src_ip','ip_addr_family','dst_ip_mask','action_tunnel_id','tunnel_type','ip_addr_family','dst_ip','type','vr_id','ip_addr_family','src_ip_mask' ]
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
		    src_ip_ptr = new_sai_ip_address_tp()
		    src_ip_ptr.addr_family = eval(args[1])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_mask_ptr = new_sai_ip_address_tp()
		    dst_ip_mask_ptr.addr_family = eval(args[3])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_mask_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[8] = args[8].replace(".",":").replace(",",":")
		    postList = args[8].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_ptr = new_sai_ip_address_tp()
		    dst_ip_ptr.addr_family = eval(args[7])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[12] = args[12].replace(".",":").replace(",",":")
		    postList = args[12].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_ip_mask_ptr = new_sai_ip_address_tp()
		    src_ip_mask_ptr.addr_family = eval(args[11])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_mask_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_tunnel_term_table_entry_default(int(args[0]),src_ip_ptr,dst_ip_mask_ptr,int(args[5]),eval(args[6]),dst_ip_ptr,eval(args[9]),int(args[10]),src_ip_mask_ptr)
		    delete_sai_ip_address_tp(src_ip_ptr)
		    delete_sai_ip_address_tp(dst_ip_mask_ptr)
		    delete_sai_ip_address_tp(dst_ip_ptr)
		    delete_sai_ip_address_tp(src_ip_mask_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, ip_addr_family, src_ip, ip_addr_family, dst_ip_mask, action_tunnel_id, tunnel_type, ip_addr_family, dst_ip, type, vr_id, ip_addr_family, src_ip_mask'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ip_addr_family','src_ip','ip_addr_family','dst_ip_mask','action_tunnel_id','tunnel_type','ip_addr_family','dst_ip','type','vr_id','ip_addr_family','src_ip_mask' ]
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
		    src_ip_ptr = new_sai_ip_address_tp()
		    src_ip_ptr.addr_family = eval(args[1])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_mask_ptr = new_sai_ip_address_tp()
		    dst_ip_mask_ptr.addr_family = eval(args[3])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_mask_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[8] = args[8].replace(".",":").replace(",",":")
		    postList = args[8].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_ip_ptr = new_sai_ip_address_tp()
		    dst_ip_ptr.addr_family = eval(args[7])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    dst_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[12] = args[12].replace(".",":").replace(",",":")
		    postList = args[12].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_ip_mask_ptr = new_sai_ip_address_tp()
		    src_ip_mask_ptr.addr_family = eval(args[11])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    src_ip_mask_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_tunnel_term_table_entry_custom(int(args[0]),src_ip_ptr,dst_ip_mask_ptr,int(args[5]),eval(args[6]),dst_ip_ptr,eval(args[9]),int(args[10]),src_ip_mask_ptr)
		    delete_sai_ip_address_tp(src_ip_ptr)
		    delete_sai_ip_address_tp(dst_ip_mask_ptr)
		    delete_sai_ip_address_tp(dst_ip_ptr)
		    delete_sai_ip_address_tp(src_ip_mask_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_tunnel_map
    #/*********************************************************/
    def do_sai_create_tunnel_map(self, arg):

        ''' sai_create_tunnel_map '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_tunnel_map_default(int(args[0]),eval(args[1]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_tunnel_map_custom(int(args[0]),eval(args[1]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_tunnel_attribute
    #/*********************************************************/
    def do_sai_set_tunnel_attribute(self, arg):

	'''sai_set_tunnel_attribute: Enter[ tunnel_id, attr_id, attr_value ]'''

	varStr = 'tunnel_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TUNNEL_ATTR_OVERLAY_INTERFACE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_PEER_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_DECAP_ECN_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_TTL_VAL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_DECAP_MAPPERS'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_DST_IP'):
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
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_SRC_IP'):
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
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_DECAP_TTL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_TERM_TABLE_ENTRY_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_MAPPERS'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_LOOPBACK_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_TTL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_DECAP_DSCP_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_GRE_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_ATTR_ENCAP_ECN_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tunnel_map_entry_attribute
    #/*********************************************************/
    def do_sai_set_tunnel_map_entry_attribute(self, arg):

	'''sai_set_tunnel_map_entry_attribute: Enter[ tunnel_map_entry_id, attr_id, attr_value ]'''

	varStr = 'tunnel_map_entry_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_entry_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_map_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tunnel_term_table_entry_attribute
    #/*********************************************************/
    def do_sai_set_tunnel_term_table_entry_attribute(self, arg):

	'''sai_set_tunnel_term_table_entry_attribute: Enter[ tunnel_term_table_entry_id, attr_id, attr_value ]'''

	varStr = 'tunnel_term_table_entry_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_term_table_entry_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_IP_ADDR_FAMILY'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP'):
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
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP_MASK'):
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
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP'):
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
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP_MASK'):
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
			ret = sai_set_tunnel_term_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_tunnel_map_attribute
    #/*********************************************************/
    def do_sai_set_tunnel_map_attribute(self, arg):

	'''sai_set_tunnel_map_attribute: Enter[ tunnel_map_id, attr_id, attr_value ]'''

	varStr = 'tunnel_map_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_TUNNEL_MAP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_tunnel_map_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_TUNNEL_MAP_ATTR_ENTRY_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_tunnel_map_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_tunnel_attribute
    #/*********************************************************/
    def do_sai_get_tunnel_attribute(self, arg):

	'''sai_get_tunnel_attribute: Enter[  tunnel_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tunnel_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tunnel_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tunnel_map_entry_attribute
    #/*********************************************************/
    def do_sai_get_tunnel_map_entry_attribute(self, arg):

	'''sai_get_tunnel_map_entry_attribute: Enter[  tunnel_map_entry_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tunnel_map_entry_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_entry_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tunnel_map_entry_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tunnel_term_table_entry_attribute
    #/*********************************************************/
    def do_sai_get_tunnel_term_table_entry_attribute(self, arg):

	'''sai_get_tunnel_term_table_entry_attribute: Enter[  tunnel_term_table_entry_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tunnel_term_table_entry_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_term_table_entry_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tunnel_term_table_entry_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_tunnel_map_attribute
    #/*********************************************************/
    def do_sai_get_tunnel_map_attribute(self, arg):

	'''sai_get_tunnel_map_attribute: Enter[  tunnel_map_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' tunnel_map_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_tunnel_map_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_tunnel_stats
    #/*********************************************************/
    def do_sai_get_tunnel_stats(self, arg):

	'''sai_get_tunnel_stats: Enter[  tunnel_id, number_of_counters ]'''

	varStr = ' tunnel_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id','number_of_counters' ]
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
		ret = sai_get_tunnel_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_tunnel_stats
    #/*********************************************************/
    def do_sai_clear_tunnel_stats(self, arg):

	'''sai_clear_tunnel_stats: Enter[  tunnel_id, number_of_counters ]'''

	varStr = ' tunnel_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id','number_of_counters' ]
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
		ret = sai_clear_tunnel_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_tunnel_stats_ext
    #/*********************************************************/
    def do_sai_get_tunnel_stats_ext(self, arg):

	'''sai_get_tunnel_stats_ext: Enter[  tunnel_id, number_of_counters, mode ]'''

	varStr = ' tunnel_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id','number_of_counters','mode' ]
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
		ret = sai_get_tunnel_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_tunnel
    #/*********************************************************/
    def do_sai_remove_tunnel(self, arg):

	'''sai_remove_tunnel: Enter[  tunnel_id ]'''

	varStr = ' tunnel_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tunnel(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tunnel_map_entry
    #/*********************************************************/
    def do_sai_remove_tunnel_map_entry(self, arg):

	'''sai_remove_tunnel_map_entry: Enter[  tunnel_map_entry_id ]'''

	varStr = ' tunnel_map_entry_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_entry_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tunnel_map_entry(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tunnel_term_table_entry
    #/*********************************************************/
    def do_sai_remove_tunnel_term_table_entry(self, arg):

	'''sai_remove_tunnel_term_table_entry: Enter[  tunnel_term_table_entry_id ]'''

	varStr = ' tunnel_term_table_entry_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_term_table_entry_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tunnel_term_table_entry(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_tunnel_map
    #/*********************************************************/
    def do_sai_remove_tunnel_map(self, arg):

	'''sai_remove_tunnel_map: Enter[  tunnel_map_id ]'''

	varStr = ' tunnel_map_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'tunnel_map_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_tunnel_map(int(args[0]))
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
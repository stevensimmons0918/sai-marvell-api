#!/usr/bin/env python
#  saiShellNeighbor.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellNeighbor.py
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
# The class object for saiShellNeighbor operations
#/**********************************************************************************/

class saiShellNeighborObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_neighbor_entry
    #/*********************************************************/
    def do_sai_create_neighbor_entry(self, arg):

        ''' sai_create_neighbor_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = 'switch_id, rif_id, ip_addr_family, ip_addr, dst_mac_address'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','rif_id','ip_addr_family','ip_addr','dst_mac_address' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ip_addr_ptr = new_sai_ip_address_tp()
		    ip_addr_ptr.addr_family = eval(args[2])
		    args[3] = args[3].replace(".",":").replace(",",":")
		    postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    dst_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_neighbor_entry_default(int(args[0]), int(args[1]), ip_addr_ptr,dst_mac_address_ptr)
		    delete_sai_ip_address_tp(ip_addr_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = 'switch_id, rif_id, ip_addr_family, ip_addr, packet_action, dst_mac_address, no_host_route'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','rif_id','ip_addr_family','ip_addr','packet_action','dst_mac_address','no_host_route' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ip_addr_ptr = new_sai_ip_address_tp()
		    ip_addr_ptr.addr_family = eval(args[2])
		    args[3] = args[3].replace(".",":").replace(",",":")
		    postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    dst_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    dst_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_neighbor_entry_custom(int(args[0]), int(args[1]), ip_addr_ptr,eval(args[4]),dst_mac_address_ptr,int(args[6]))
		    delete_sai_ip_address_tp(ip_addr_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_neighbor_entry_attribute
    #/*********************************************************/
    def do_sai_set_neighbor_entry_attribute(self, arg):

	'''sai_set_neighbor_entry_attribute: Enter[ switch_id, rif_id, ip_addr_family, ip_addr, attr_id, attr_value ]'''

	varStr = 'switch_id, rif_id, ip_addr_family, ip_addr, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','rif_id','ip_addr_family','ip_addr','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ip_addr_ptr = new_sai_ip_address_tp()
		ip_addr_ptr.addr_family = eval(args[2])
		args[3] = args[3].replace(".",":").replace(",",":")
		postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		args[4]=eval(args[4])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_IS_LOCAL'):
			args[5]= int(args[5])
			attr_value_ptr.u8= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_META_DATA'):
			args[5]= int(args[5])
			attr_value_ptr.u32= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[5])
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_USER_TRAP_ID'):
			args[5]= int(args[5])
			attr_value_ptr.oid= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_IP_ADDR_FAMILY'):
			attr_value_ptr.s32= eval(args[5])
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_ENCAP_INDEX'):
			args[5]= int(args[5])
			attr_value_ptr.u32= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS'):
			args[5] = args[5].replace(".",":").replace(",",":")
			postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE'):
			args[5]= int(args[5])
			attr_value_ptr.u8= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_COUNTER_ID'):
			args[5]= int(args[5])
			attr_value_ptr.oid= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		elif args[4] == eval('SAI_NEIGHBOR_ENTRY_ATTR_ENCAP_IMPOSE_INDEX'):
			args[5]= int(args[5])
			attr_value_ptr.u8= args[5]
			ret = sai_set_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(ip_addr_ptr)
		else:
			print("Invalid attr_id "+str(args[4])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_neighbor_entry_attribute
    #/*********************************************************/
    def do_sai_get_neighbor_entry_attribute(self, arg):

	'''sai_get_neighbor_entry_attribute: Enter[ switch_id, rif_id, ip_addr_family, ip_addr, attr_id, count_if_attribute_is_list ]'''

	varStr = 'switch_id, rif_id, ip_addr_family, ip_addr, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','rif_id','ip_addr_family','ip_addr','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ip_addr_ptr = new_sai_ip_address_tp()
		ip_addr_ptr.addr_family = eval(args[2])
		args[3] = args[3].replace(".",":").replace(",",":")
		postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		ret = sai_get_neighbor_entry_attribute(int(args[0]), int(args[1]), ip_addr_ptr, eval(args[4]), int(args[5]),None)
		delete_sai_ip_address_tp(ip_addr_ptr)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_neighbor_entry
    #/*********************************************************/
    def do_sai_remove_neighbor_entry(self, arg):

	'''sai_remove_neighbor_entry: Enter[ switch_id, rif_id, ip_addr_family, ip_addr ]'''

	varStr = 'switch_id, rif_id, ip_addr_family, ip_addr'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','rif_id','ip_addr_family','ip_addr' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ip_addr_ptr = new_sai_ip_address_tp()
		ip_addr_ptr.addr_family = eval(args[2])
		args[3] = args[3].replace(".",":").replace(",",":")
		postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		ret = sai_remove_neighbor_entry(int(args[0]), int(args[1]), ip_addr_ptr)
		delete_sai_ip_address_tp(ip_addr_ptr)
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
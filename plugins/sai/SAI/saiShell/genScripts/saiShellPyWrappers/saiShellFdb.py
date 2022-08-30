#!/usr/bin/env python
#  saiShellFdb.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellFdb.py
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
# The class object for saiShellFdb operations
#/**********************************************************************************/

class saiShellFdbObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_fdb_entry
    #/*********************************************************/
    def do_sai_create_fdb_entry(self, arg):

        ''' sai_create_fdb_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, mac_address, bv_id, packet_action, ip_addr_family, endpoint_ip, bridge_port_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','mac_address','bv_id','packet_action','ip_addr_family','endpoint_ip','bridge_port_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:

		    sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()
		    sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])
		    args[1] = args[1].replace(".",":").replace(",",":")
		    postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    for ix in range(0, listLen, 1): 
			    sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)
		    args[2]=int(args[2])
		    sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    endpoint_ip_ptr = new_sai_ip_address_tp()
		    endpoint_ip_ptr.addr_family = eval(args[4])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    endpoint_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_fdb_entry_default(sai_fdb_entry_t_fdb_entry_ptr,eval(args[3]),endpoint_ip_ptr,int(args[6]),eval(args[7]))
		    delete_sai_ip_address_tp(endpoint_ip_ptr)
		    delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, mac_address, bv_id, packet_action, ip_addr_family, endpoint_ip, bridge_port_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','mac_address','bv_id','packet_action','ip_addr_family','endpoint_ip','bridge_port_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:

		    sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()
		    sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])
		    args[1] = args[1].replace(".",":").replace(",",":")
		    postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    for ix in range(0, listLen, 1): 
			    sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)
		    args[2]=int(args[2])
		    sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    endpoint_ip_ptr = new_sai_ip_address_tp()
		    endpoint_ip_ptr.addr_family = eval(args[4])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    endpoint_ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_fdb_entry_custom(sai_fdb_entry_t_fdb_entry_ptr,eval(args[3]),endpoint_ip_ptr,int(args[6]),eval(args[7]))
		    delete_sai_ip_address_tp(endpoint_ip_ptr)
		    delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_fdb_entry_attribute
    #/*********************************************************/
    def do_sai_set_fdb_entry_attribute(self, arg):

	'''sai_set_fdb_entry_attribute: Enter[  switch_id, mac_address, bv_id, attr_id, attr_value ]'''

	varStr = ' switch_id, mac_address, bv_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','mac_address','bv_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:

		sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()
		sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])
		args[1] = args[1].replace(".",":").replace(",",":")
		postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		for ix in range(0, listLen, 1): 
			sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)
		args[2]=int(args[2])
		sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]
		args[3]=eval(args[3])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[3] == eval('SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE'):
			args[4]= int(args[4])
			attr_value_ptr.u8= args[4]
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_META_DATA'):
			args[4]= int(args[4])
			attr_value_ptr.u32= args[4]
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[4])
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_ENDPOINT_IP'):
			ipaddrflag = 1
			args[5] = args[5].replace(".",":").replace(",",":")
			postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			ipaddr_ptr.addr_family = eval(args[4])
			base = 10
			if listLen > 4:
				base = 16
			for ix in range(0, listLen, 1):
				ipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_COUNTER_ID'):
			args[4]= int(args[4])
			attr_value_ptr.oid= args[4]
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID'):
			args[4]= int(args[4])
			attr_value_ptr.oid= args[4]
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[4])
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		elif args[3] == eval('SAI_FDB_ENTRY_ATTR_USER_TRAP_ID'):
			args[4]= int(args[4])
			attr_value_ptr.oid= args[4]
			ret = sai_set_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		else:
			print("Invalid attr_id "+str(args[3])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_fdb_entry_attribute
    #/*********************************************************/
    def do_sai_get_fdb_entry_attribute(self, arg):

	'''sai_get_fdb_entry_attribute: Enter[  switch_id, mac_address, bv_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' switch_id, mac_address, bv_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','mac_address','bv_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:

		sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()
		sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])
		args[1] = args[1].replace(".",":").replace(",",":")
		postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		for ix in range(0, listLen, 1): 
			sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)
		args[2]=int(args[2])
		sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]
		ret = sai_get_fdb_entry_attribute(sai_fdb_entry_t_fdb_entry_ptr, eval(args[3]), int(args[4]),None)
		delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_fdb_entry
    #/*********************************************************/
    def do_sai_remove_fdb_entry(self, arg):

	'''sai_remove_fdb_entry: Enter[  switch_id, mac_address, bv_id ]'''

	varStr = ' switch_id, mac_address, bv_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','mac_address','bv_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:

		sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()
		sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])
		args[1] = args[1].replace(".",":").replace(",",":")
		postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		for ix in range(0, listLen, 1): 
			sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)
		args[2]=int(args[2])
		sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]
		ret = sai_remove_fdb_entry(sai_fdb_entry_t_fdb_entry_ptr)
		delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)
		xpShellGlobals.cmdRetVal = long(ret)

    def do_sai_flush_fdb_entries(self, arg):
		'''sai_flush_fdb_entries: Enter[  switch_id SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID,port_id SAI_FDB_FLUSH_ATTR_BV_ID,vlan_id SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,attr_type ]'''
		varList = arg.split(' ')
		switchId = varList[0]
		attr_value_arr = new_arr_sai_attribute_tp(len(varList) - 1)
		attr_value_ptr = new_sai_attribute_tp()
		count = 0
		if(len(varList) <= 1):
			print('Invalid input, Enter [  switch_id SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID,port_id SAI_FDB_FLUSH_ATTR_BV_ID,vlan_id SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,attr_type ]');
		for i in range(1, len(varList)):
			attr = varList[i].replace(' ', '').split(',')
			if(len(attr) != 2):
				print 'Invalid Arguemnt', varList[i]
				count = 0
				break
			else:
				if(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID):
					attr_value_ptr.value.oid = int(attr[1])
				elif(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_BV_ID):
					attr_value_ptr.value.oid = int(attr[1])
				elif(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_ENTRY_TYPE):
					attr_value_ptr.value.s32 = int(attr[1])
				else:
					print 'Invalid attribute type:', attr[0]
					count = 0
					break
				attr_value_ptr.id = eval(attr[0])
				arr_sai_attribute_tp_setitem(attr_value_arr, (i - 1),sai_attribute_tp_value(attr_value_ptr))
				count = count + 1

		if(count != 0):
			ret = sai_flush_fdb_entries(int(switchId), count, attr_value_arr)
			print 'sai_flush_fdb_entries returned reason code:', ret
		delete_sai_attribute_tp(attr_value_ptr)
		delete_arr_sai_attribute_tp(attr_value_arr)

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
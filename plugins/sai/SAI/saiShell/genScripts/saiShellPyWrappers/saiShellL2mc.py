#!/usr/bin/env python
#  saiShellL2mc.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellL2mc.py
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
# The class object for saiShellL2mc operations
#/**********************************************************************************/

class saiShellL2mcObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_l2mc_entry
    #/*********************************************************/
    def do_sai_create_l2mc_entry(self, arg):

        ''' sai_create_l2mc_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, packet_action'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','bv_id','type','ipaddr_family','src_ip','dst_ip','packet_action' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    src_ip_addr_ptr = new_sai_ip_address_tp()
		    src_ip_addr_ptr.addr_family = eval(args[3])
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    dst_ip_addr_ptr = new_sai_ip_address_tp()
		    dst_ip_addr_ptr.addr_family = eval(args[3])
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_l2mc_entry_default(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, eval(args[6]))
		    delete_sai_ip_address_tp(src_ip_addr_ptr)
		    delete_sai_ip_address_tp(dst_ip_addr_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, output_group_id, packet_action'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','bv_id','type','ipaddr_family','src_ip','dst_ip','output_group_id','packet_action' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    src_ip_addr_ptr = new_sai_ip_address_tp()
		    src_ip_addr_ptr.addr_family = eval(args[3])
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    dst_ip_addr_ptr = new_sai_ip_address_tp()
		    dst_ip_addr_ptr.addr_family = eval(args[3])
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1): 
			    dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_l2mc_entry_custom(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, int(args[6]),eval(args[7]))
		    delete_sai_ip_address_tp(src_ip_addr_ptr)
		    delete_sai_ip_address_tp(dst_ip_addr_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_l2mc_entry_attribute
    #/*********************************************************/
    def do_sai_set_l2mc_entry_attribute(self, arg):

	'''sai_set_l2mc_entry_attribute: Enter[  switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, attr_id, attr_value ]'''

	varStr = ' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','bv_id','type','ipaddr_family','src_ip','dst_ip','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		src_ip_addr_ptr = new_sai_ip_address_tp()
		src_ip_addr_ptr.addr_family = eval(args[3])
		args[4] = args[4].replace(".",":").replace(",",":")
		postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		dst_ip_addr_ptr = new_sai_ip_address_tp()
		dst_ip_addr_ptr.addr_family = eval(args[3])
		args[5] = args[5].replace(".",":").replace(",",":")
		postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		args[6]=eval(args[6])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[6] == eval('SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID'):
			args[7]= int(args[7])
			attr_value_ptr.oid= args[7]
			ret = sai_set_l2mc_entry_attribute(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, args[6], attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(src_ip_addr_ptr)
			delete_sai_ip_address_tp(dst_ip_addr_ptr)
		elif args[6] == eval('SAI_L2MC_ENTRY_ATTR_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[7])
			ret = sai_set_l2mc_entry_attribute(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, args[6], attr_value_ptr, int(ipaddrflag), ipaddr_ptr)
			delete_sai_ip_address_tp(src_ip_addr_ptr)
			delete_sai_ip_address_tp(dst_ip_addr_ptr)
		else:
			print("Invalid attr_id "+str(args[6])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_l2mc_entry_attribute
    #/*********************************************************/
    def do_sai_get_l2mc_entry_attribute(self, arg):

	'''sai_get_l2mc_entry_attribute: Enter[  switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, attr_id, count_if_attribute_is_list ]'''

	varStr = ' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','bv_id','type','ipaddr_family','src_ip','dst_ip','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		src_ip_addr_ptr = new_sai_ip_address_tp()
		src_ip_addr_ptr.addr_family = eval(args[3])
		args[4] = args[4].replace(".",":").replace(",",":")
		postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		dst_ip_addr_ptr = new_sai_ip_address_tp()
		dst_ip_addr_ptr.addr_family = eval(args[3])
		args[5] = args[5].replace(".",":").replace(",",":")
		postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		ret = sai_get_l2mc_entry_attribute(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, eval(args[6]), int(args[7]),None)
		delete_sai_ip_address_tp(src_ip_addr_ptr)
		delete_sai_ip_address_tp(dst_ip_addr_ptr)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_l2mc_entry
    #/*********************************************************/
    def do_sai_remove_l2mc_entry(self, arg):

	'''sai_remove_l2mc_entry: Enter[  switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip ]'''

	varStr = ' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'switch_id','bv_id','type','ipaddr_family','src_ip','dst_ip' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		src_ip_addr_ptr = new_sai_ip_address_tp()
		src_ip_addr_ptr.addr_family = eval(args[3])
		args[4] = args[4].replace(".",":").replace(",",":")
		postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		dst_ip_addr_ptr = new_sai_ip_address_tp()
		dst_ip_addr_ptr.addr_family = eval(args[3])
		args[5] = args[5].replace(".",":").replace(",",":")
		postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		listLen = len(postList)
		base = 10
		if listLen > 4:
			base = 16
		for ix in range(0, listLen, 1): 
			dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)
		ret = sai_remove_l2mc_entry(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr)
		delete_sai_ip_address_tp(src_ip_addr_ptr)
		delete_sai_ip_address_tp(dst_ip_addr_ptr)
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
#!/usr/bin/env python
#  saiShellNat.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellNat.py
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
# The class object for saiShellNat operations
#/**********************************************************************************/

class saiShellNatObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_nat_entry
    #/*********************************************************/
    def do_sai_create_nat_entry(self, arg):

        ''' sai_create_nat_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' nat_entry'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'nat_entry' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_nat_entry_default(eval(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' nat_entry'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'nat_entry' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_nat_entry_custom(eval(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_nat_zone_counter
    #/*********************************************************/
    def do_sai_create_nat_zone_counter(self, arg):

        ''' sai_create_nat_zone_counter '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_nat_zone_counter_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_nat_zone_counter_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_nat_entry_attribute
    #/*********************************************************/
    def do_sai_set_nat_entry_attribute(self, arg):

	'''sai_set_nat_entry_attribute: Enter[ nat_entry, attr_id, attr_value ]'''

	varStr = 'nat_entry, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_entry','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_NAT_ENTRY_ATTR_ENABLE_BYTE_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_VR_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_L4_SRC_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_HIT_BIT_COR'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_ENABLE_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_SRC_IP'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			attr_value_ptr.ip4 = 0
			for ix in range(listLen): 
				attr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_DST_IP_MASK'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			attr_value_ptr.ip4 = 0
			for ix in range(listLen): 
				attr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_HIT_BIT'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_SRC_IP_MASK'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			attr_value_ptr.ip4 = 0
			for ix in range(listLen): 
				attr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_DST_IP'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			attr_value_ptr.ip4 = 0
			for ix in range(listLen): 
				attr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_NAT_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_L4_DST_PORT'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_BYTE_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ENTRY_ATTR_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_nat_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_nat_zone_counter_attribute
    #/*********************************************************/
    def do_sai_set_nat_zone_counter_attribute(self, arg):

	'''sai_set_nat_zone_counter_attribute: Enter[ nat_zone_counter_id, attr_id, attr_value ]'''

	varStr = 'nat_zone_counter_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_zone_counter_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_ZONE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_NAT_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_TRANSLATION_NEEDED_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_TRANSLATIONS_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_ENABLE_TRANSLATION_NEEDED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_DISCARD_PACKET_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u64= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_ENABLE_TRANSLATIONS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NAT_ZONE_COUNTER_ATTR_ENABLE_DISCARD'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_nat_zone_counter_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_nat_entry_attribute
    #/*********************************************************/
    def do_sai_get_nat_entry_attribute(self, arg):

	'''sai_get_nat_entry_attribute: Enter[  nat_entry, attr_id, count_if_attribute_is_list ]'''

	varStr = ' nat_entry, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_entry','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_nat_entry_attribute(eval(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_nat_zone_counter_attribute
    #/*********************************************************/
    def do_sai_get_nat_zone_counter_attribute(self, arg):

	'''sai_get_nat_zone_counter_attribute: Enter[  nat_zone_counter_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' nat_zone_counter_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_zone_counter_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_nat_zone_counter_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_nat_entry
    #/*********************************************************/
    def do_sai_remove_nat_entry(self, arg):

	'''sai_remove_nat_entry: Enter[  nat_entry ]'''

	varStr = ' nat_entry'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_entry' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_nat_entry(eval(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_nat_zone_counter
    #/*********************************************************/
    def do_sai_remove_nat_zone_counter(self, arg):

	'''sai_remove_nat_zone_counter: Enter[  nat_zone_counter_id ]'''

	varStr = ' nat_zone_counter_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'nat_zone_counter_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_nat_zone_counter(int(args[0]))
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
#!/usr/bin/env python
#  saiShellMymac.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellMymac.py
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
# The class object for saiShellMymac operations
#/**********************************************************************************/

class saiShellMymacObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_my_mac
    #/*********************************************************/
    def do_sai_create_my_mac(self, arg):

        ''' sai_create_my_mac '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, mac_address_mask, port_id, mac_address, vlan_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','mac_address_mask','port_id','mac_address','vlan_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[1] = args[1].replace(".",":").replace(",",":")
		    postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    mac_address_mask_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    mac_address_mask_ptr.mac[ix] =  int(postList[ix], 16)
		    args[3] = args[3].replace(".",":").replace(",",":")
		    postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_my_mac_default(int(args[0]),mac_address_mask_ptr,int(args[2]),mac_address_ptr,int(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, mac_address_mask, port_id, priority, mac_address, vlan_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','mac_address_mask','port_id','priority','mac_address','vlan_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[1] = args[1].replace(".",":").replace(",",":")
		    postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    mac_address_mask_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    mac_address_mask_ptr.mac[ix] =  int(postList[ix], 16)
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_my_mac_custom(int(args[0]),mac_address_mask_ptr,int(args[2]),int(args[3]),mac_address_ptr,int(args[5]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_my_mac_attribute
    #/*********************************************************/
    def do_sai_set_my_mac_attribute(self, arg):

	'''sai_set_my_mac_attribute: Enter[ my_mac_id, attr_id, attr_value ]'''

	varStr = 'my_mac_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'my_mac_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_MY_MAC_ATTR_MAC_ADDRESS_MASK'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_my_mac_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MY_MAC_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_my_mac_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MY_MAC_ATTR_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_my_mac_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MY_MAC_ATTR_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_my_mac_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_MY_MAC_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_my_mac_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_my_mac_attribute
    #/*********************************************************/
    def do_sai_get_my_mac_attribute(self, arg):

	'''sai_get_my_mac_attribute: Enter[  my_mac_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' my_mac_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'my_mac_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_my_mac_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_my_mac
    #/*********************************************************/
    def do_sai_remove_my_mac(self, arg):

	'''sai_remove_my_mac: Enter[  my_mac_id ]'''

	varStr = ' my_mac_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'my_mac_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_my_mac(int(args[0]))
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
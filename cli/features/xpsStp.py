#!/usr/bin/env python
#  xpsStp.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

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
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsStp operations
#/**********************************************************************************/

class xpsStpObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsStpInitScope
    #/********************************************************************************/
    def do_stp_init_scope(self, arg):
        '''
         xpsStpInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsStpInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsStpDeInitScope
    #/********************************************************************************/
    def do_stp_de_init_scope(self, arg):
        '''
         xpsStpDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsStpDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpInit
    #/********************************************************************************/
    def do_stp_init(self, arg):
        '''
         xpsStpInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsStpInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpDeInit
    #/********************************************************************************/
    def do_stp_de_init(self, arg):
        '''
         xpsStpDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsStpDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpAddDevice
    #/********************************************************************************/
    def do_stp_add_device(self, arg):
        '''
         xpsStpAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsStpAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpRemoveDevice
    #/********************************************************************************/
    def do_stp_remove_device(self, arg):
        '''
         xpsStpRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsStpRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpSetDefault
    #/********************************************************************************/
    def do_stp_set_default(self, arg):
        '''
         xpsStpSetDefault: Enter [ devId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, stpId=%d' % (args[0],args[1]))
            ret = xpsStpSetDefault(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpGetDefault
    #/********************************************************************************/
    def do_stp_get_default(self, arg):
        '''
         xpsStpGetDefault: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            stpId_Ptr_1 = new_xpsStp_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsStpGetDefault(args[0],stpId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsStp_tp_value(stpId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsStp_tp_value(stpId_Ptr_1)))
                pass
            delete_xpsStp_tp(stpId_Ptr_1)

    #/********************************************************************************/
    # command for xpsStpCreateScope
    #/********************************************************************************/
    def do_stp_create_scope(self, arg):
        '''
         xpsStpCreateScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            stpId_Ptr_1 = new_xpsStp_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsStpCreateScope(args[0],stpId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsStp_tp_value(stpId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsStp_tp_value(stpId_Ptr_1))
                pass
            delete_xpsStp_tp(stpId_Ptr_1)
    #/********************************************************************************/
    # command for xpsStpReserveScope
    #/********************************************************************************/
    def do_stp_reserve_scope(self, arg):
        '''
         xpsStpReserveScope: Enter [ scopeId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, stpId=%d' % (args[0],args[1]))
            ret = xpsStpReserveScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsStpDestroyScope
    #/********************************************************************************/
    def do_stp_destroy_scope(self, arg):
        '''
         xpsStpDestroyScope: Enter [ scopeId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, stpId=%d' % (args[0],args[1]))
            ret = xpsStpDestroyScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpCreate
    #/********************************************************************************/
    def do_stp_create(self, arg):
        '''
         xpsStpCreate: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            stpId_Ptr_0 = new_xpsStp_tp()
            #print('Input Arguments are' % ())
            ret = xpsStpCreate(stpId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsStp_tp_value(stpId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsStp_tp_value(stpId_Ptr_0)))
                pass
            delete_xpsStp_tp(stpId_Ptr_0)

    #/********************************************************************************/
    # command for xpsStpReserve
    #/********************************************************************************/
    def do_stp_reserve(self, arg):
        '''
         xpsStpReserve: Enter [ stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ stpId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, stpId=%d' % (args[0]))
            ret = xpsStpReserve(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpDestroy
    #/********************************************************************************/
    def do_stp_destroy(self, arg):
        '''
         xpsStpDestroy: Enter [ stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ stpId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, stpId=%d' % (args[0]))
            ret = xpsStpDestroy(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # Auto completion for vlanStgState enumerations
    #/********************************************************************************/
    def complete_stp_set_state(self, text, line, begidx, endidx):
        tempDict = { 4 : 'vlanStgState'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsStpSetState
    #/********************************************************************************/
    def do_stp_set_state(self, arg):
        '''
         xpsStpSetState: Enter [ devId,stpId,intfId,stpState ]
         Valid values for stpState  : <vlanStgState>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,stpId,intfId,stpState ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, stpId=%d, intfId=%d, stpState=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsStpSetState(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsStpGetState
    #/********************************************************************************/
    def do_stp_get_state(self, arg):
        '''
         xpsStpGetState: Enter [ devId,stpId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,stpId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            stpState_Ptr_3 = new_xpsStpState_ep()
            #print('Input Arguments are, devId=%d, stpId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsStpGetState(args[0],args[1],args[2],stpState_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpState = %d' % (xpsStpState_ep_value(stpState_Ptr_3)))
                pass
            delete_xpsStpState_ep(stpState_Ptr_3)

    #/********************************************************************************/
    # command for xpsStgGetVlanList
    #/********************************************************************************/
    def do_stp_get_vlan_list(self, arg):
        '''
         xpsStgGetVlanList: Enter [ devId,StpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,StpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vlanList_Ptr_2 = new_xpsVlan_tpp(1)
            vlanList_Ptr_1 = new_arrUint16(16)
            xpsVlan_tpp_setitem(vlanList_Ptr_2, 0, vlanList_Ptr_1)
            numOfVlans_Ptr_3 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsStgGetVlanList(args[0],args[1],vlanList_Ptr_2,numOfVlans_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                vlanList = []
                for i in range(0, uint16_tp_value(numOfVlans_Ptr_3)):
                    vlanList.append(arrUint16_getitem(xpsVlan_tpp_getitem(vlanList_Ptr_2, 0), i))
                if not vlanList:
                    print('numOfVlans = %d' % (uint16_tp_value(numOfVlans_Ptr_3)))
                else:
                    print('vlanList = %s' % (str(vlanList).replace('L', '').replace('[', "").replace(']', "")))
                    print('numOfVlans = %d' % (uint16_tp_value(numOfVlans_Ptr_3)))
                pass
            delete_uint16_tp(numOfVlans_Ptr_3)
            delete_arrUint16(vlanList_Ptr_1)
            delete_xpsVlan_tpp(vlanList_Ptr_2)

    #/*****************************************************************************************************/
    # sub-commands to display tables
    #/*****************************************************************************************************/
    def do_display_tables(self, s):
        'Display Table commands'
        i = displayTableCmds()
        i.prompt = self.prompt[:-1]+':displayTables)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i


#!/usr/bin/env python
#  xpsCopp.py
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

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsCopp operations
#/**********************************************************************************/

class xpsCoppObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsCoppInit
    #/********************************************************************************/
    def do_copp_init(self, arg):
        '''
         xpsCoppInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsCoppInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppInitScope
    #/********************************************************************************/
    def do_copp_init_scope(self, arg):
        '''
         xpsCoppInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsCoppInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppDeInit
    #/********************************************************************************/
    def do_copp_de_init(self, arg):
        '''
         xpsCoppDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsCoppDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppDeInitScope
    #/********************************************************************************/
    def do_copp_de_init_scope(self, arg):
        '''
         xpsCoppDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsCoppDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppAddDevice
    #/********************************************************************************/
    def do_copp_add_device(self, arg):
        '''
         xpsCoppAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsCoppAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppRemoveDevice
    #/********************************************************************************/
    def do_copp_remove_device(self, arg):
        '''
         xpsCoppRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsCoppRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppAddEntry
    #/********************************************************************************/
    def do_copp_add_entry(self, arg):
        '''
         xpsCoppAddEntry: Enter [ devId,sourcePort,reasonCode,enPolicer,policerId,updatePktCmd,pktCmd,updateReasonCode,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sourcePort,reasonCode,enPolicer,policerId,updatePktCmd,pktCmd,updateReasonCode,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpCoppEntryData_t_Ptr = xpCoppEntryData_t()
            xpCoppEntryData_t_Ptr.enPolicer = int(args[3])
            xpCoppEntryData_t_Ptr.policerId = int(args[4])
            xpCoppEntryData_t_Ptr.updatePktCmd = int(args[5])
            xpCoppEntryData_t_Ptr.pktCmd = int(args[6])
            xpCoppEntryData_t_Ptr.updateReasonCode = int(args[7])
            xpCoppEntryData_t_Ptr.reasonCode = int(args[8])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, sourcePort=%d, reasonCode=%d, enPolicer=%d, policerId=%d, updatePktCmd=%d, pktCmd=%d, updateReasonCode=%d, reasonCode=%d' % (args[0],args[1],args[2],xpCoppEntryData_t_Ptr.enPolicer,xpCoppEntryData_t_Ptr.policerId,xpCoppEntryData_t_Ptr.updatePktCmd,xpCoppEntryData_t_Ptr.pktCmd,xpCoppEntryData_t_Ptr.updateReasonCode,xpCoppEntryData_t_Ptr.reasonCode))
            ret = xpsCoppAddEntry(args[0],args[1],args[2],xpCoppEntryData_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
    #/********************************************************************************/
    # command for xpsCoppUpdateEntry
    #/********************************************************************************/
    def do_copp_update_entry(self, arg):
        '''
         xpsCoppUpdateEntry: Enter [ devId,index,enPolicer,policerId,updatePktCmd,pktCmd,updateReasonCode,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,enPolicer,policerId,updatePktCmd,pktCmd,updateReasonCode,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpCoppEntryData_t_Ptr = xpCoppEntryData_t()
            xpCoppEntryData_t_Ptr.enPolicer = int(args[2])
            xpCoppEntryData_t_Ptr.policerId = int(args[3])
            xpCoppEntryData_t_Ptr.updatePktCmd = int(args[4])
            xpCoppEntryData_t_Ptr.pktCmd = int(args[5])
            xpCoppEntryData_t_Ptr.updateReasonCode = int(args[6])
            xpCoppEntryData_t_Ptr.reasonCode = int(args[7])
            #print('Input Arguments are, devId=%d, index=%d, enPolicer=%d, policerId=%d, updatePktCmd=%d, pktCmd=%d, updateReasonCode=%d, reasonCode=%d' % (args[0],args[1],xpCoppEntryData_t_Ptr.enPolicer,xpCoppEntryData_t_Ptr.policerId,xpCoppEntryData_t_Ptr.updatePktCmd,xpCoppEntryData_t_Ptr.pktCmd,xpCoppEntryData_t_Ptr.updateReasonCode,xpCoppEntryData_t_Ptr.reasonCode))
            ret = xpsCoppUpdateEntry(args[0],args[1],xpCoppEntryData_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppGetIndex
    #/********************************************************************************/
    def do_copp_get_index(self, arg):
        '''
         xpsCoppGetIndex: Enter [ devId,sourcePort,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sourcePort,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            index_Ptr_3 = new_intp()
            #print('Input Arguments are, devId=%d, sourcePort=%d, reasonCode=%d' % (args[0],args[1],args[2]))
            ret = xpsCoppGetIndex(args[0],args[1],args[2],index_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (intp_value(index_Ptr_3)))
                pass
            delete_intp(index_Ptr_3)
    #/********************************************************************************/
    # command for xpsCoppGetEntry
    #/********************************************************************************/
    def do_copp_get_entry(self, arg):
        '''
         xpsCoppGetEntry: Enter [ devId,sourcePort,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sourcePort,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpCoppEntryData_t_Ptr = new_xpCoppEntryData_tp()
            #print('Input Arguments are, devId=%d, sourcePort=%d, reasonCode=%d' % (args[0],args[1],args[2]))
            ret = xpsCoppGetEntry(args[0],args[1],args[2],xpCoppEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enPolicerId = %d' % (xpCoppEntryData_t_Ptr.enPolicer))
                print('policerId = %d' % (xpCoppEntryData_t_Ptr.policerId))
                print('updatePktCmd = %d' % (xpCoppEntryData_t_Ptr.updatePktCmd))
                print('pktCmd = %d' % (xpCoppEntryData_t_Ptr.pktCmd))
                print('updateReasonCode = %d' % (xpCoppEntryData_t_Ptr.updateReasonCode))
                print('reasonCode = %d' % (xpCoppEntryData_t_Ptr.reasonCode))
                pass
            delete_xpCoppEntryData_tp(xpCoppEntryData_t_Ptr)
    #/********************************************************************************/
    # command for xpsCoppGetEntryByIndex
    #/********************************************************************************/
    def do_copp_get_entry_by_index(self, arg):
        '''
         xpsCoppGetEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpCoppEntryData_t_Ptr = new_xpCoppEntryData_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsCoppGetEntryByIndex(args[0],args[1],xpCoppEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enPolicerId = %d' % (xpCoppEntryData_t_Ptr.enPolicer))
                print('policerId = %d' % (xpCoppEntryData_t_Ptr.policerId))
                print('updatePktCmd = %d' % (xpCoppEntryData_t_Ptr.updatePktCmd))
                print('pktCmd = %d' % (xpCoppEntryData_t_Ptr.pktCmd))
                print('updateReasonCode = %d' % (xpCoppEntryData_t_Ptr.updateReasonCode))
                print('reasonCode = %d' % (xpCoppEntryData_t_Ptr.reasonCode))
                pass
            delete_xpCoppEntryData_tp(xpCoppEntryData_t_Ptr)
    #/********************************************************************************/
    # command for xpsCoppRemoveEntry
    #/********************************************************************************/
    def do_copp_remove_entry(self, arg):
        '''
         xpsCoppRemoveEntry: Enter [ devId,sourcePort,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sourcePort,reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, sourcePort=%d, reasonCode=%d' % (args[0],args[1],args[2]))
            ret = xpsCoppRemoveEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsCoppRemoveEntryByIndex
    #/********************************************************************************/
    def do_copp_remove_entry_by_index(self, arg):
        '''
         xpsCoppRemoveEntryByIndex: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsCoppRemoveEntryByIndex(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
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
    #/********************************************************************************/
    # command for xpsCoppEnableCtrlPktTrapOnPort
    #/********************************************************************************/
    def do_copp_enable_ctrl_pkt_trap_on_port(self, arg):
        '''
         xpsCoppEnableCtrlPktyTrapOnPort: Enter [ devId, type, port, enable]
        '''
        print arg;
        args = re.split(';| ',arg)
        numArgsReq = 4 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, type, port, enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            enable  = int(args[3])
            #print('Input Arguments are, devId=%d, type = %d, port=%d, enable=%d' % (args[0],args[1],args[2],enable))
            ret = xpsCoppEnableCtrlPktTrapOnPort(args[0],args[1],args[2],enable)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
    #/********************************************************************************/
    # command for xpsCoppEnableCtrlPktTrapOnVlan
    #/********************************************************************************/
    def do_copp_enable_ctrl_pkt_trap_on_vlan(self, arg):
        '''
         xpsCoppEnableCtrlPktyTrapOnVlan: Enter [ devId, type, vlan, enable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, type, vlan, enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            enable  = int(args[3])
            #print('Input Arguments are, devId=%d, type = %d, vlan=%d, enable=%d' % (args[0],args[1],args[2],enable))
            ret = xpsCoppEnableCtrlPktTrapOnVlan(args[0],args[1],args[2],arg[3])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
    #/********************************************************************************/
    # command for xpsCoppEnableCtrlPktTrapOnSwitch
    #/********************************************************************************/
    def do_copp_enable_ctrl_pkt_trap_on_switch(self, arg):
        '''
         xpsCoppEnableCtrlPktyTrapOnSwitch: Enter [ devId, type, pktCmd]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, type, pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, type = %d, enable=%d' % (args[0],args[1],enable))
            ret = xpsCoppEnableCtrlPktTrapOnSwitch(args[0],args[1],args[2])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
    #/********************************************************************************/
    # command for xpsCoppDisplayCtrlPktTraps
    #/********************************************************************************/
    def do_copp_display_ctrl_pkt_traps(self, arg):
        '''
         xpsCoppDisplayCtrlPktTraps: Enter [ devId, type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, type ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, type = %d' % (args[0],args[1])
            ret = xpsCoppDisplayCtrlPktTraps(args[0],args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass

#!/usr/bin/env python
#  xpsPolicer.py
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
# The class object for xpsPolicer operations
#/**********************************************************************************/

class xpsPolicerObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsPolicerInitScope
    #/********************************************************************************/
    def do_policer_init_scope(self, arg):
        '''
         xpsPolicerInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsPolicerInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerInit
    #/********************************************************************************/
    def do_policer_init(self, arg):
        '''
         xpsPolicerInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPolicerInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPolicerDeInitScope
    #/********************************************************************************/
    def do_policer_de_init_scope(self, arg):
        '''
         xpsPolicerDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsPolicerDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsPolicerDeInit
    #/********************************************************************************/
    def do_policer_de_init(self, arg):
        '''
         xpsPolicerDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPolicerDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerAddDevice
    #/********************************************************************************/
    def do_policer_add_device(self, arg):
        '''
         xpsPolicerAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsPolicerAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerRemoveDevice
    #/********************************************************************************/
    def do_policer_remove_device(self, arg):
        '''
         xpsPolicerRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPolicerRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsPolicerEnablePortPolicingScope
    #/********************************************************************************/
    def do_policer_enable_port_policing_scope(self, arg):
        '''
         xpsPolicerEnablePortPolicingScope: Enter [ scopeId,portIntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerEnablePortPolicingScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsPolicerEnablePortPolicing
    #/********************************************************************************/
    def do_policer_enable_port_policing(self, arg):
        '''
         xpsPolicerEnablePortPolicing: Enter [ portIntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, portIntfId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsPolicerEnablePortPolicing(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_add_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerAddEntry
    #/********************************************************************************/
    def do_policer_add_entry(self, arg):
        '''
         xpsPolicerAddEntry: Enter [ devId,client,index,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult,isPacketBasedPolicin ]
        Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 15
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult,isPacketBasedPolicin ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            xpsPolicerEntry_t_Ptr = new_xpsPolicerEntry_tp()
            xpsPolicerEntry_t_Ptr.cir = int(args[3])
            xpsPolicerEntry_t_Ptr.pir = int(args[4])
            xpsPolicerEntry_t_Ptr.cbs = int(args[5])
            xpsPolicerEntry_t_Ptr.pbs = int(args[6])
            xpsPolicerEntry_t_Ptr.colorAware = int(args[7])
            xpsPolicerEntry_t_Ptr.dropRed = int(args[8])
            xpsPolicerEntry_t_Ptr.dropYellow = int(args[9])
            xpsPolicerEntry_t_Ptr.updateResultRed = int(args[10])
            xpsPolicerEntry_t_Ptr.updateResultYellow = int(args[11])
            xpsPolicerEntry_t_Ptr.updateResultGreen = int(args[12])
            xpsPolicerEntry_t_Ptr.polResult = int(args[13])
            xpsPolicerEntry_t_Ptr.isPacketBasedPolicing = int(args[14])
            
            #print('Input Arguments are, devId=%d, client=%d, index=%d, cir=%d, pir=%d, cbs=%d, pbs=%d, colorAware=%d, dropRed=%d, dropYellow=%d, updateResultRed=%d, updateResultYellow=%d, updateResultGreen=%d, polResult=%d' % (args[0],args[1],args[2],xpsPolicerEntry_t_Ptr.cir,xpsPolicerEntry_t_Ptr.pir,xpsPolicerEntry_t_Ptr.cbs,xpsPolicerEntry_t_Ptr.pbs,xpsPolicerEntry_t_Ptr.colorAware,xpsPolicerEntry_t_Ptr.dropRed,xpsPolicerEntry_t_Ptr.dropYellow,xpsPolicerEntry_t_Ptr.updateResultRed,xpsPolicerEntry_t_Ptr.updateResultYellow,xpsPolicerEntry_t_Ptr.updateResultGreen,xpsPolicerEntry_t_Ptr.polResult))
            ret = xpsPolicerAddEntry(args[0],args[1],args[2],xpsPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cir = %d' % (xpsPolicerEntry_t_Ptr.cir))
                print('pir = %d' % (xpsPolicerEntry_t_Ptr.pir))
                print('cbs = %d' % (xpsPolicerEntry_t_Ptr.cbs))
                print('pbs = %d' % (xpsPolicerEntry_t_Ptr.pbs))
                print('colorAware = %d' % (xpsPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpsPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpsPolicerEntry_t_Ptr.dropYellow))
                print('updateResultRed = %d' % (xpsPolicerEntry_t_Ptr.updateResultRed))
                print('updateResultYellow = %d' % (xpsPolicerEntry_t_Ptr.updateResultYellow))
                print('updateResultGreen = %d' % (xpsPolicerEntry_t_Ptr.updateResultGreen))
                print('polResult = %d' % (xpsPolicerEntry_t_Ptr.polResult))
                print('isPacketBasedPolicing = %d' % (xpsPolicerEntry_t_Ptr.isPacketBasedPolicing))
                pass
            delete_xpsPolicerEntry_tp(xpsPolicerEntry_t_Ptr)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_remove_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerRemoveEntry
    #/********************************************************************************/
    def do_policer_remove_entry(self, arg):
        '''
         xpsPolicerRemoveEntry: Enter [ devId,client,index ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerRemoveEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_get_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerGetEntry
    #/********************************************************************************/
    def do_policer_get_entry(self, arg):
        '''
         xpsPolicerGetEntry: Enter [ devId,client,index ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            xpsPolicerEntry_t_Ptr = new_xpsPolicerEntry_tp()            
            ret = xpsPolicerGetEntry(args[0],args[1],args[2],xpsPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cir = %d' % (xpsPolicerEntry_t_Ptr.cir))
                print('pir = %d' % (xpsPolicerEntry_t_Ptr.pir))
                print('cbs = %d' % (xpsPolicerEntry_t_Ptr.cbs))
                print('pbs = %d' % (xpsPolicerEntry_t_Ptr.pbs))
                print('colorAware = %d' % (xpsPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpsPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpsPolicerEntry_t_Ptr.dropYellow))
                print('updateResultRed = %d' % (xpsPolicerEntry_t_Ptr.updateResultRed))
                print('updateResultYellow = %d' % (xpsPolicerEntry_t_Ptr.updateResultYellow))
                print('updateResultGreen = %d' % (xpsPolicerEntry_t_Ptr.updateResultGreen))
                print('polResult = %d' % (xpsPolicerEntry_t_Ptr.polResult))
                print('isPacketBasedPolicing = %d' % (xpsPolicerEntry_t_Ptr.isPacketBasedPolicing))
                pass
            delete_xpsPolicerEntry_tp(xpsPolicerEntry_t_Ptr)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_get_policer_counter_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerGetPolicerCounterEntry
    #/********************************************************************************/
    def do_policer_get_policer_counter_entry(self, arg):
        '''
         xpsPolicerGetPolicerCounterEntry: Enter [ devId,client,index ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            pEntry_Ptr_3 = new_xpsPolicerCounterEntry_tp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerGetPolicerCounterEntry(args[0],args[1],args[2],pEntry_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Green Events = %d' % (pEntry_Ptr_3.greenEvent))
                print('Green Sum = %d' % (pEntry_Ptr_3.greenSum))
                print('Yellow Events = %d' % (pEntry_Ptr_3.yellowEvent))
                print('Yellow Sum = %d' % (pEntry_Ptr_3.yellowSum))
                print('Red Events = %d' % (pEntry_Ptr_3.redEvent))
                print('Red Sum = %d' % (pEntry_Ptr_3.redSum))
                pass
            delete_xpsPolicerCounterEntry_tp(pEntry_Ptr_3)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_add_port_policing_entry_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerAddPortPolicingEntryScope
    #/********************************************************************************/
    def do_policer_add_port_policing_entry_scope(self, arg):
        '''
         xpsPolicerAddPortPolicingEntryScope: Enter [ scopeId,portIntfId,client,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 14
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,client,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpsPolicerEntry_t_Ptr = new_xpsPolicerEntry_tp()
            xpsPolicerEntry_t_Ptr.cir = int(args[3])
            xpsPolicerEntry_t_Ptr.pir = int(args[4])
            xpsPolicerEntry_t_Ptr.cbs = int(args[5])
            xpsPolicerEntry_t_Ptr.pbs = int(args[6])
            xpsPolicerEntry_t_Ptr.colorAware = int(args[7])
            xpsPolicerEntry_t_Ptr.dropRed = int(args[8])
            xpsPolicerEntry_t_Ptr.dropYellow = int(args[9])
            xpsPolicerEntry_t_Ptr.updateResultRed = int(args[10])
            xpsPolicerEntry_t_Ptr.updateResultYellow = int(args[11])
            xpsPolicerEntry_t_Ptr.updateResultGreen = int(args[12])
            xpsPolicerEntry_t_Ptr.polResult = int(args[13])
            
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, client=%d, cir=%d, pir=%d, cbs=%d, pbs=%d, colorAware=%d, dropRed=%d, dropYellow=%d, updateResultRed=%d, updateResultYellow=%d, updateResultGreen=%d, polResult=%d' % (args[0],args[1],args[2],xpsPolicerEntry_t_Ptr.cir,xpsPolicerEntry_t_Ptr.pir,xpsPolicerEntry_t_Ptr.cbs,xpsPolicerEntry_t_Ptr.pbs,xpsPolicerEntry_t_Ptr.colorAware,xpsPolicerEntry_t_Ptr.dropRed,xpsPolicerEntry_t_Ptr.dropYellow,xpsPolicerEntry_t_Ptr.updateResultRed,xpsPolicerEntry_t_Ptr.updateResultYellow,xpsPolicerEntry_t_Ptr.updateResultGreen,xpsPolicerEntry_t_Ptr.polResult))
            ret = xpsPolicerAddPortPolicingEntryScope(args[0],args[1],args[2],xpsPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cir = %d' % (xpsPolicerEntry_t_Ptr.cir))
                print('pir = %d' % (xpsPolicerEntry_t_Ptr.pir))
                print('cbs = %d' % (xpsPolicerEntry_t_Ptr.cbs))
                print('pbs = %d' % (xpsPolicerEntry_t_Ptr.pbs))
                print('colorAware = %d' % (xpsPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpsPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpsPolicerEntry_t_Ptr.dropYellow))
                print('updateResultRed = %d' % (xpsPolicerEntry_t_Ptr.updateResultRed))
                print('updateResultYellow = %d' % (xpsPolicerEntry_t_Ptr.updateResultYellow))
                print('updateResultGreen = %d' % (xpsPolicerEntry_t_Ptr.updateResultGreen))
                print('polResult = %d' % (xpsPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpsPolicerEntry_tp(xpsPolicerEntry_t_Ptr)
			
    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_add_port_policing_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerAddPortPolicingEntry
    #/********************************************************************************/
    def do_policer_add_port_policing_entry(self, arg):
        '''
         xpsPolicerAddPortPolicingEntry: Enter [ portIntfId,client,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 13
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,client,cir,pir,cbs,pbs,colorAware,dropRed,dropYellow,updateResultRed,updateResultYellow,updateResultGreen,polResult ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpsPolicerEntry_t_Ptr = new_xpsPolicerEntry_tp()
            xpsPolicerEntry_t_Ptr.cir = int(args[2])
            xpsPolicerEntry_t_Ptr.pir = int(args[3])
            xpsPolicerEntry_t_Ptr.cbs = int(args[4])
            xpsPolicerEntry_t_Ptr.pbs = int(args[5])
            xpsPolicerEntry_t_Ptr.colorAware = int(args[6])
            xpsPolicerEntry_t_Ptr.dropRed = int(args[7])
            xpsPolicerEntry_t_Ptr.dropYellow = int(args[8])
            xpsPolicerEntry_t_Ptr.updateResultRed = int(args[9])
            xpsPolicerEntry_t_Ptr.updateResultYellow = int(args[10])
            xpsPolicerEntry_t_Ptr.updateResultGreen = int(args[11])
            xpsPolicerEntry_t_Ptr.polResult = int(args[12])
            
            #print('Input Arguments are, portIntfId=%d, client=%d, cir=%d, pir=%d, cbs=%d, pbs=%d, colorAware=%d, dropRed=%d, dropYellow=%d, updateResultRed=%d, updateResultYellow=%d, updateResultGreen=%d, polResult=%d' % (args[0],args[1],xpsPolicerEntry_t_Ptr.cir,xpsPolicerEntry_t_Ptr.pir,xpsPolicerEntry_t_Ptr.cbs,xpsPolicerEntry_t_Ptr.pbs,xpsPolicerEntry_t_Ptr.colorAware,xpsPolicerEntry_t_Ptr.dropRed,xpsPolicerEntry_t_Ptr.dropYellow,xpsPolicerEntry_t_Ptr.updateResultRed,xpsPolicerEntry_t_Ptr.updateResultYellow,xpsPolicerEntry_t_Ptr.updateResultGreen,xpsPolicerEntry_t_Ptr.polResult))
            ret = xpsPolicerAddPortPolicingEntry(args[0],args[1],xpsPolicerEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cir = %d' % (xpsPolicerEntry_t_Ptr.cir))
                print('pir = %d' % (xpsPolicerEntry_t_Ptr.pir))
                print('cbs = %d' % (xpsPolicerEntry_t_Ptr.cbs))
                print('pbs = %d' % (xpsPolicerEntry_t_Ptr.pbs))
                print('colorAware = %d' % (xpsPolicerEntry_t_Ptr.colorAware))
                print('dropRed = %d' % (xpsPolicerEntry_t_Ptr.dropRed))
                print('dropYellow = %d' % (xpsPolicerEntry_t_Ptr.dropYellow))
                print('updateResultRed = %d' % (xpsPolicerEntry_t_Ptr.updateResultRed))
                print('updateResultYellow = %d' % (xpsPolicerEntry_t_Ptr.updateResultYellow))
                print('updateResultGreen = %d' % (xpsPolicerEntry_t_Ptr.updateResultGreen))
                print('polResult = %d' % (xpsPolicerEntry_t_Ptr.polResult))
                pass
            delete_xpsPolicerEntry_tp(xpsPolicerEntry_t_Ptr)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_remove_port_policing_entry_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerRemovePortPolicingEntryScope
    #/********************************************************************************/
    def do_policer_remove_port_policing_entry_scope(self, arg):
        '''
         xpsPolicerRemovePortPolicingEntryScope: Enter [ scopeId,portIntfId,client ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,client ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, client=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerRemovePortPolicingEntryScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_remove_port_policing_entry(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerRemovePortPolicingEntry
    #/********************************************************************************/
    def do_policer_remove_port_policing_entry(self, arg):
        '''
         xpsPolicerRemovePortPolicingEntry: Enter [ portIntfId,client ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,client ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, portIntfId=%d, client=%d' % (args[0],args[1]))
            ret = xpsPolicerRemovePortPolicingEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_set_attribute(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerSetAttribute
    #/********************************************************************************/
    def do_policer_set_attribute(self, arg):
        '''
         xpsPolicerSetAttribute: Enter [ devId,client,index,field ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            data_Ptr_4 = new_voidp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d, field=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPolicerSetAttribute(args[0],args[1],args[2],args[3],data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_4)))
                pass
            delete_voidp(data_Ptr_4)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_get_attribute(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerGetAttribute
    #/********************************************************************************/
    def do_policer_get_attribute(self, arg):
        '''
         xpsPolicerGetAttribute: Enter [ devId,client,index,field ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,field ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            data_Ptr_4 = new_voidp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d, field=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPolicerGetAttribute(args[0],args[1],args[2],args[3],data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (voidp_value(data_Ptr_4)))
                pass
            delete_voidp(data_Ptr_4)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_set_result_by_type(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerSetResultByType
    #/********************************************************************************/
    def do_policer_set_result_by_type(self, arg):
        '''
         xpsPolicerSetResultByType: Enter [ devId,client,index,ResultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 22
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,resultType,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp,dp,tc,pcp,dei,dscp,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            xpPolicerResult_t_Ptr_red = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr_red.dp = int(args[4])
            xpPolicerResult_t_Ptr_red.tc = int(args[5])
            xpPolicerResult_t_Ptr_red.pcp = int(args[6])
            xpPolicerResult_t_Ptr_red.dei = int(args[7])
            xpPolicerResult_t_Ptr_red.dscp = int(args[8])
            xpPolicerResult_t_Ptr_red.exp = int(args[9])
            xpPolicerResult_t_Ptr_yel = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr_yel.dp = int(args[10])
            xpPolicerResult_t_Ptr_yel.tc = int(args[11])
            xpPolicerResult_t_Ptr_yel.pcp = int(args[12])
            xpPolicerResult_t_Ptr_yel.dei = int(args[13])
            xpPolicerResult_t_Ptr_yel.dscp = int(args[14])
            xpPolicerResult_t_Ptr_yel.exp = int(args[15])
            xpPolicerResult_t_Ptr_gre = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr_gre.dp = int(args[16])
            xpPolicerResult_t_Ptr_gre.tc = int(args[17])
            xpPolicerResult_t_Ptr_gre.pcp = int(args[18])
            xpPolicerResult_t_Ptr_gre.dei = int(args[19])
            xpPolicerResult_t_Ptr_gre.dscp = int(args[20])
            xpPolicerResult_t_Ptr_gre.exp = int(args[21])

            #print('Input Arguments are, devId=%d, client=%d, index=%d, resultType=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d' % (args[0],args[1],args[2],xpPolicerResult_t_Ptr_red.dp,xpPolicerResult_t_Ptr_red.tc,xpPolicerResult_t_Ptr_red.pcp,xpPolicerResult_t_Ptr_red.dei,xpPolicerResult_t_Ptr_red.dscp,xpPolicerResult_t_Ptr_red.exp,xpPolicerResult_t_Ptr_yel.dp,xpPolicerResult_t_Ptr_yel.tc,xpPolicerResult_t_Ptr_yel.pcp,xpPolicerResult_t_Ptr_yel.dei,xpPolicerResult_t_Ptr_yel.dscp,xpPolicerResult_t_Ptr_yel.exp,xpPolicerResult_t_Ptr_gre.dp,xpPolicerResult_t_Ptr_gre.tc,xpPolicerResult_t_Ptr_gre.pcp,xpPolicerResult_t_Ptr_gre.dei,xpPolicerResult_t_Ptr_gre.dscp,xpPolicerResult_t_Ptr_gre.exp))
            ret = xpsPolicerSetResultByType(args[0],args[1],args[2],args[3],xpPolicerResult_t_Ptr_red,xpPolicerResult_t_Ptr_yel,xpPolicerResult_t_Ptr_gre)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr_red.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_red.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_red.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_red.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_red.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_red.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr_yel.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_yel.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_yel.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_yel.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_yel.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_yel.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr_gre.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_gre.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_gre.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_gre.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_gre.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_gre.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_red)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_yel)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_gre)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_policer_get_result_by_type(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsPolicerGetResultByType
    #/********************************************************************************/
    def do_policer_get_result_by_type(self, arg):
        '''
         xpsPolicerGetResultByType: Enter [ devId,client,index,ResultType ]
         Valid values for client  : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,ResultType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            xpPolicerResult_t_Ptr_red = new_xpPolicerResult_tp()                       
            xpPolicerResult_t_Ptr_yel = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr_gre = new_xpPolicerResult_tp()
            ret = xpsPolicerGetResultByType(args[0],args[1],args[2],args[3],xpPolicerResult_t_Ptr_red,xpPolicerResult_t_Ptr_yel,xpPolicerResult_t_Ptr_gre)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr_red.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_red.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_red.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_red.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_red.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_red.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr_yel.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_yel.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_yel.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_yel.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_yel.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_yel.exp))
                print('dp = %d' % (xpPolicerResult_t_Ptr_gre.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr_gre.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr_gre.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr_gre.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr_gre.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr_gre.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_red)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_yel)
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr_gre)
    #/********************************************************************************/
    # command for xpsPolicerSetResultByColor
    #/********************************************************************************/
    def do_policer_set_result_by_color(self, arg):
        '''
         xpsPolicerSetResultByColor: Enter [ devId,client,index,resultType,color,dp,tc,pcp,dei,dscp,exp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 11
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,resultType,color,dp,tc,pcp,dei,dscp,exp ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            xpPolicerResult_t_Ptr = new_xpPolicerResult_tp()
            xpPolicerResult_t_Ptr.dp = int(args[5])
            xpPolicerResult_t_Ptr.tc = int(args[6])
            xpPolicerResult_t_Ptr.pcp = int(args[7])
            xpPolicerResult_t_Ptr.dei = int(args[8])
            xpPolicerResult_t_Ptr.dscp = int(args[9])
            xpPolicerResult_t_Ptr.exp = int(args[10])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, resultType=%d, color=%d, dp=%d, tc=%d, pcp=%d, dei=%d, dscp=%d, exp=%d' % (args[0],args[1],args[2],args[3],args[4],xpPolicerResult_t_Ptr.dp,xpPolicerResult_t_Ptr.tc,xpPolicerResult_t_Ptr.pcp,xpPolicerResult_t_Ptr.dei,xpPolicerResult_t_Ptr.dscp,xpPolicerResult_t_Ptr.exp))
            ret = xpsPolicerSetResultByColor(args[0],args[1],args[2],args[3],args[4],xpPolicerResult_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('dp = %d' % (xpPolicerResult_t_Ptr.dp))
                print('tc = %d' % (xpPolicerResult_t_Ptr.tc))
                print('pcp = %d' % (xpPolicerResult_t_Ptr.pcp))
                print('dei = %d' % (xpPolicerResult_t_Ptr.dei))
                print('dscp = %d' % (xpPolicerResult_t_Ptr.dscp))
                print('exp = %d' % (xpPolicerResult_t_Ptr.exp))
                pass
            delete_xpPolicerResult_tp(xpPolicerResult_t_Ptr)

    #/********************************************************************************/
    # command for xpsPolicerSetPolicerStandard
    #/********************************************************************************/
    def do_policer_set_policer_standard(self, arg):
        '''
         xpsPolicerSetPolicerStandard: Enter [ devId,polStandard ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,polStandard ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, polStandard=%d' % (args[0],args[1]))
            ret = xpsPolicerSetPolicerStandard(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerGetPolicerStandard
    #/********************************************************************************/
    def do_policer_get_policer_standard(self, arg):
        '''
         xpsPolicerGetPolicerStandard: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            polStandard_Ptr_1 = new_xpPolicerStandard_ep()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPolicerGetPolicerStandard(args[0],polStandard_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polStandard = %d' % (xpPolicerStandard_ep_value(polStandard_Ptr_1)))
                pass
            delete_xpPolicerStandard_ep(polStandard_Ptr_1)
    #/********************************************************************************/
    # command for xpsPolicerEnablePacketBasedPolicing
    #/********************************************************************************/
    def do_policer_enable_packet_based_policing(self, arg):
        '''
         xpsPolicerEnablePacketBasedPolicing: Enter [ devId,client,index,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPolicerEnablePacketBasedPolicing(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerIsPacketBasedPolicingEnabled
    #/********************************************************************************/
    def do_policer_is_packet_based_policing_enabled(self, arg):
        '''
         xpsPolicerIsPacketBasedPolicingEnabled: Enter [ devId,client,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerIsPacketBasedPolicingEnabled(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsPolicerEnableByteBasedPolicing
    #/********************************************************************************/
    def do_policer_enable_byte_based_policing(self, arg):
        '''
         xpsPolicerEnableByteBasedPolicing: Enter [ devId,client,index,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPolicerEnableByteBasedPolicing(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPolicerIsByteBasedPolicingEnabled
    #/********************************************************************************/
    def do_policer_is_byte_based_policing_enabled(self, arg):
        '''
         xpsPolicerIsByteBasedPolicingEnabled: Enter [ devId,client,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsPolicerIsByteBasedPolicingEnabled(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
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


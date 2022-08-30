#!/usr/bin/env python
#  xpsMirror.py
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
# The class object for xpsMirror operations
#/**********************************************************************************/

class xpsMirrorObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsMirrorInitScope
    #/********************************************************************************/
    def do_mirror_init_scope(self, arg):
        '''
         xpsMirrorInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMirrorInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsMirrorInit
    #/********************************************************************************/
    def do_mirror_init(self, arg):
        '''
         xpsMirrorInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMirrorInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorDeInitScope
    #/********************************************************************************/
    def do_mirror_de_init_scope(self, arg):
        '''
         xpsMirrorDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMirrorDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorDeInit
    #/********************************************************************************/
    def do_mirror_de_init(self, arg):
        '''
         xpsMirrorDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMirrorDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for initType enumerations
    #/********************************************************************************/
    def complete_mirror_add_device(self, text, line, begidx, endidx):
        tempDict = { 2 : 'initType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsMirrorAddDevice
    #/********************************************************************************/
    def do_mirror_add_device(self, arg):
        '''
         xpsMirrorAddDevice: Enter [ devId,initType ]
         Valid values for initType : <initType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsMirrorAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorRemoveDevice
    #/********************************************************************************/
    def do_mirror_remove_device(self, arg):
        '''
         xpsMirrorRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMirrorRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorCreateAnalyzerSessionScope
    #/********************************************************************************/
    def do_mirror_create_analyzer_session_scope(self, arg):
        '''
         xpsMirrorCreateAnalyzerSessionScope: Enter [ scopeId,type,data,dir ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,type,data,dir ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            arg2 = xpMirrorData_t()
            arg2.erspan2Data.erspanId = int(args[2])
            args[3] = int(args[3])
            analyzerId_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, type=%d, data=0x%x, dir=%d' % (args[0],args[1],arg2.erspan2Data.erspanId,args[3]))
            ret = xpsMirrorCreateAnalyzerSessionScope(args[0],args[1],arg2,args[3],analyzerId_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('analyzerId = %d' % (uint32_tp_value(analyzerId_Ptr_4)))
                pass
            delete_uint32_tp(analyzerId_Ptr_4)

    #/********************************************************************************/
    # command for xpsMirrorCreateAnalyzerSession
    #/********************************************************************************/
    def do_mirror_create_analyzer_session(self, arg):
        '''
         xpsMirrorCreateAnalyzerSession: Enter [ type,data,dir ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ type,data,dir ]\n For SPAN/RSPAN : type = 0/2, data=0 \n For ERSPAN : type = 1, data <=1023\n dir = 0 for egress and 1 for ingress')
        else:
            args[0] = eval(args[0])
            arg1 = xpMirrorData_t()
            if((args[0] == XP_MIRROR_LOCAL) or (args[0] == XP_MIRROR_ERSPAN2) or (args[0] == XP_MIRROR_RSPAN)):
                #print "Mirror Type : ",str(args[0])	
                arg1.erspan2Data.erspanId = int(args[1])
            #elif:
            #   #Need to add when we support rspan and erspan3 
            #   pass
            else:
                #Invalid input, error should be returned from API.
                print "Invalid Mirror type"
                pass

            args[2] = eval(args[2])
            analyzerId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, type=%d, data=0x%x, dir=%d' % (args[0],args[1],args[2]))
            ret = xpsMirrorCreateAnalyzerSession(args[0], arg1, args[2],analyzerId_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('analyzerId = %d' % (uint32_tp_value(analyzerId_Ptr_3)))
                pass
            delete_uint32_tp(analyzerId_Ptr_3)

    #/********************************************************************************/
    # command for xpsMirrorAddAnalyzerInterfaceScope
    #/********************************************************************************/
    def do_mirror_add_analyzer_interface_scope(self, arg):
        '''
         xpsMirrorAddAnalyzerInterfaceScope: Enter [ scopeId,analyzerId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,analyzerId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, analyzerId=%d, analyzerIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMirrorAddAnalyzerInterfaceScope(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorAddAnalyzerInterface
    #/********************************************************************************/
    def do_mirror_add_analyzer_interface(self, arg):
        '''
         xpsMirrorAddAnalyzerInterface: Enter [ analyzerId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ analyzerId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, analyzerId=%d, analyzerIntfId=%d' % (args[0],args[1]))
            ret = xpsMirrorAddAnalyzerInterface(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorWriteAnalyzerSession
    #/********************************************************************************/
    def do_mirror_write_analyzer_session(self, arg):
        '''
         xpsMirrorWriteAnalyzerSession: Enter [ deviceId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsMirrorWriteAnalyzerSession(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorRemoveAnalyzerSession
    #/********************************************************************************/
    def do_mirror_remove_analyzer_session(self, arg):
        '''
         xpsMirrorRemoveAnalyzerSession: Enter [ deviceId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsMirrorRemoveAnalyzerSession(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorRemoveAnalyzerInterface
    #/********************************************************************************/
    def do_mirror_remove_analyzer_interface(self, arg):
        '''
         xpsMirrorRemoveAnalyzerInterface: Enter [ deviceId,analyzerId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,analyzerId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, analyzerId=%d, analyzerIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMirrorRemoveAnalyzerInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorDestroyAnalyzerInterfaceScope
    #/********************************************************************************/
    def do_mirror_destroy_analyzer_interface_scope(self, arg):
        '''
         xpsMirrorDestroyAnalyzerInterfaceScope: Enter [ scopeId,analyzerId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,analyzerId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, analyzerId=%d, analyzerIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMirrorDestroyAnalyzerInterfaceScope(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorGetMirrorSessionId
    #/********************************************************************************/
    def do_mirror_get_mirror_session_id (self, arg):
        '''
         xpsMirrorGetMirrorSession: Enter [ analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ analyzerId]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, analyzerId=%d' % (args[0]))
            mirrorSession_id_ptr = new_uint32_tp()
            mirrorBitMask_ptr = new_uint32_tp()
            ret = xpsMirrorGetMirrorSessionId(args[0],mirrorSession_id_ptr,mirrorBitMask_ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('mirrorSessionId: %d mirrorBitMask: %d' % (uint32_tp_value(mirrorSession_id_ptr), uint32_tp_value(mirrorBitMask_ptr)))
                pass
            delete_uint32_tp(mirrorSession_id_ptr)
            delete_uint32_tp(mirrorBitMask_ptr)

    #/********************************************************************************/
    # command for xpsMirrorGetMirrorSessionIdScope
    #/********************************************************************************/
    def do_mirror_get_mirror_session_id_scope (self, arg):
        '''
         xpsMirrorGetMirrorSession: Enter [ scopeId, analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId, analyzerId]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are,  scopeId=%d nalyzerId=%d' % (args[0], args[1]))
            mirrorSession_id_ptr = new_uint32_tp()
            mirrorBitMask_ptr = new_uint32_tp()
            ret = xpsMirrorGetMirrorSessionIdScope(args[0], args[1], mirrorSession_id_ptr,mirrorBitMask_ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('mirrorSessionId: %d mirrorBitMask: %d' % (uint32_tp_value(mirrorSession_id_ptr), uint32_tp_value(mirrorBitMask_ptr)))
                pass
            delete_uint32_tp(mirrorSession_id_ptr)
            delete_uint32_tp(mirrorBitMask_ptr)

    #/********************************************************************************/
    # command for xpsMirrorDestroyAnalyzerSessionScope
    #/********************************************************************************/
    def do_mirror_destroy_analyzer_session_scope(self, arg):
        '''
         xpsMirrorDestroyAnalyzerSessionScope: Enter [ scopeId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsMirrorDestroyAnalyzerSessionScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsMirrorDestroyAnalyzerInterface
    #/********************************************************************************/
    def do_mirror_destroy_analyzer_interface(self, arg):
        '''
         xpsMirrorDestroyAnalyzerInterface: Enter [ analyzerId,analyzerIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ analyzerId,analyzerIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, analyzerId=%d, analyzerIntfId=%d' % (args[0],args[1]))
            ret = xpsMirrorDestroyAnalyzerInterface(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorDestroyAnalyzerSession
    #/********************************************************************************/
    def do_mirror_destroy_analyzer_session(self, arg):
        '''
         xpsMirrorDestroyAnalyzerSession: Enter [ analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ analyzerId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, analyzerId=%d' % (args[0]))
            ret = xpsMirrorDestroyAnalyzerSession(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMirrorGetRspanAnalyzerInterface
    #/********************************************************************************/
    def do_mirror_get_rspan_analyzer_interface(self, arg):
        '''
        xpsMirrorGetRspanAnalyzerInterface: Enter [ deviceId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            analyzerIntfId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsMirrorGetRspanAnalyzerInterface(args[0],args[1],analyzerIntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('analyzerIntfId = %d' % (uint32_tp_value(analyzerIntfId_Ptr_2)))
                pass
            delete_uint32_tp(analyzerIntfId_Ptr_2)
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


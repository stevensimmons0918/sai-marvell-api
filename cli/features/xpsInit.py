#!/usr/bin/env python
#  xpsInit.py
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
# The class object for xpsInit operations
#/**********************************************************************************/

class xpsInitObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # Auto completion for initType enumerations
    #/********************************************************************************/
    def complete_sdk_init(self, text, line, begidx, endidx):
        tempDict = { 2 : 'initType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsSdkInit
    #/********************************************************************************/
    def do_sdk_init(self, arg):
        '''
         xpsSdkInit: Enter [ rpType,initType ]
         Valid values for initType : <initType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ rpType,initType ]')
        else:
            args[0] = eval(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, rpType=%d, initType=%d' % (args[0],args[1]))
            ret = xpsSdkInit(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkDeInit
    #/********************************************************************************/
    def do_sdk_de_init(self, arg):
        '''
         xpsSdkDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for initType enumerations
    #/********************************************************************************/
    def complete_sdk_init_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'initType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsSdkInitScope
    #/********************************************************************************/
    def do_sdk_init_scope(self, arg):
        '''
         xpsSdkInitScope: Enter [ scopeId,rpType,initType ]
         Valid values for initType : <initType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,rpType,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, scopeId=%d, rpType=%d, initType=%d' % (args[0],args[1],args[2]))
            ret = xpsSdkInitScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkDeInitScope
    #/********************************************************************************/
    def do_sdk_de_init_scope(self, arg):
        '''
         xpsSdkDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsSdkDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for initType and packetInterface enumerations
    #/********************************************************************************/
    def complete_sdk_device_init(self, text, line, begidx, endidx):
        tempDict = { 2 : 'initType', 6 : 'packetInterface'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsSdkDeviceInit
    #/********************************************************************************/
    def do_sdk_device_init(self, arg):
        '''
         xpsSdkDeviceInit: Enter [ devId,initType,dalType,rtype,clkFreq,packetInterface ]
         Valid values for initType  and packetInterface: <initType packetInterface>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType,dalType,rtype,clkFreq,packetInterface ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            args[5] = eval(args[5])
            #print('Input Arguments are, devId=%d, initType=%d, dalType=%d, rtype=%d, clkFreq=%d, packetInterface=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsSdkDeviceInit(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for initType end packetInterface numerations
    #/********************************************************************************/
    def complete_sdk_dev_init(self, text, line, begidx, endidx):
        tempDict = { 2 : 'initType', 3 : 'packetInterface'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsSdkDevInit
    #/********************************************************************************/
    def do_sdk_dev_init(self, arg):
        '''
         xpsSdkDevInit Enter [ devId,initType]
		 Valid values for initType : <initType >
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            devConfigStruct_Ptr_2 = new_xpDevConfigStructp()
            #print('Input Arguments are, devId=%d, initType=%d % (args[0],args[1]))
            ret = xpsSdkDevInit(args[0],args[1],devConfigStruct_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('devConfigStruct = %d' % (xpDevConfigStructp_value(devConfigStruct_Ptr_2)))
                pass
            delete_xpDevConfigStructp(devConfigStruct_Ptr_2)
    #/********************************************************************************/
    # command for xpsProcessTerminationHandler
    #/********************************************************************************/
    def do_process_termination_handler(self, arg):
        '''
         xpsProcessTerminationHandler: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsProcessTerminationHandler()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkDevDeInit
    #/********************************************************************************/
    def do_sdk_dev_de_init(self, arg):
        '''
         xpsSdkDevDeInit: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSdkDevDeInit(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkDevLoadProfile
    #/********************************************************************************/
    def do_sdk_dev_load_profile(self, arg):
        '''
         xpsSdkDevLoadProfile: Enter [ devId,profileType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, profileType=%d' % (args[0],args[1]))
            ret = xpsSdkDevLoadProfile(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkLoggerInit
    #/********************************************************************************/
    def do_sdk_logger_init(self, arg):
        '''
         xpsSdkLoggerInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkLoggerInit()
    #/********************************************************************************/
    # command for xpsSdkSetLoggerOptions
    #/********************************************************************************/
    def do_sdk_set_logger_options(self, arg):
        '''
         xpsSdkSetLoggerOptions: Enter [ id, logLevel]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ id,logLevel]')
        else:
            args[0] = int(args[0])
            logLevel_Ptr_1 = args[1]
            #print('Input Arguments are, id=%d' % (args[0]))
            ret = xpsSdkSetLoggerOptions(args[0],logLevel_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
    #/********************************************************************************/
    # command for xpsSdkLogConsole
    #/********************************************************************************/
    def do_sdk_log_console(self, arg):
        '''
         xpsSdkLogConsole: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkLogConsole()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkDisableLogConsole
    #/********************************************************************************/
    def do_sdk_disable_log_console(self, arg):
        '''
         xpsSdkDisableLogConsole: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkDisableLogConsole()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkLogToFile
    #/********************************************************************************/
    def do_sdk_log_to_file(self, arg):
        '''
         xpsSdkLogToFile: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkLogToFile()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSdkSetModuleLogLevel
    #/********************************************************************************/
    def do_sdk_set_module_log_level(self, arg):
        '''
         xpsSdkSetModuleLogLevel: Enter [ moduleName,logLevel]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [moduleName,logLevel]')
        else:
            moduleName_Ptr_0 = args[0]
            logLevel_Ptr_1 = args[1]
            #print('Input Arguments are' % ())
            ret = xpsSdkSetModuleLogLevel(moduleName_Ptr_0,logLevel_Ptr_1)
            if ret != 0:
                print('Return Value = %d' % (ret))

    #/********************************************************************************/
    # command for xpsSdkEnableOrDisableLogModule
    #/********************************************************************************/
    def do_sdk_enable_or_disable_module_log_console(self, arg):
        '''
         xpsSdkEnableOrDisableLogModule: Enter [ moduleName,enable/disable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [moduleName,enable/disable]')
        else:
            moduleName_Ptr_0 = args[0]
            args[1] = int(args[1])
            #print('Input Arguments are' % ())
            ret = xpsSdkEnableOrDisableLogModule(moduleName_Ptr_0, args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))

    #/********************************************************************************/
    # command for xpsSdkLogVersion
    #/********************************************************************************/
    def do_sdk_log_version(self, arg):
        '''
         xpsSdkLogVersion: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSdkLogVersion()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsTransFifoRdrBegin
    #/********************************************************************************/
    def do_trans_fifo_rdr_begin(self, arg):
        '''
         xpsTransFifoRdrBegin: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTransFifoRdrBegin(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsTransFifoRdrEnd
    #/********************************************************************************/
    def do_trans_fifo_rdr_end(self, arg):
        '''
         xpsTransFifoRdrEnd: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTransFifoRdrEnd(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSetInitType
    #/********************************************************************************/
    def do_set_init_type(self, arg):
        '''
         xpsSetInitType: Enter [ initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ initType ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, initType=%d' % (args[0]))
            ret = xpsSetInitType(args[0])
    #/********************************************************************************/
    # command for xpsGetInitType
    #/********************************************************************************/
    def do_get_init_type(self, arg):
        '''
         xpsGetInitType: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsGetInitType()
    #/********************************************************************************/
    # command for xpsIsDevInitDone
    #/********************************************************************************/
    def do_is_dev_init_done(self, arg):
        '''
         xpsIsDevInitDone: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIsDevInitDone(args[0])
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
    #/*****************************************************************************************************/
    # command to get Port Tune and Auto Negotiation thread log status
    #/*****************************************************************************************************/
    def do_port_tune_and_an_thread_log_status_get(self, arg):
        '''
        Display Port Tune and Auto Negotiation thread log status.
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [moduleName]')
        else:
            statusPtr = new_uint8_tp()
            ret = xpsSdkThreadLogStatusGet(args[0], XP_LOG_PORT_TUNE_AND_AN_THREAD_BLOCK, statusPtr)
            if (ret == 0 and uint8_tp_value(statusPtr) == 0):
                print('Port tune and Auto Negotiation thread log status :Enable')
            elif (ret == 0 and uint8_tp_value(statusPtr) == 1):
                print('Port tune and Auto Negotiation thread log status :Disable')
        delete_uint8_tp(statusPtr)

    #/*****************************************************************************************************/
    # command to set Port Tune and Auto Negotiation thread log status
    #/*****************************************************************************************************/
    def do_port_tune_and_an_thread_log_status_enable(self, arg):
        '''
        Enable/Disable Port Tune and Auto Negotiation thread log status.
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [moduleName,enable/disable]')
        else:
            status = int(args[1])
            ret = xpsSdkThreadLogStatusSet(args[0], XP_LOG_PORT_TUNE_AND_AN_THREAD_BLOCK, not(status))
            if (ret == 0):
                print('Enable/Disable Port Tune and Auto Negotiation thread logs: Success');
            else:
                print('Enable/Disable Port Tune and Auto Negotiation thread logs: Failed');

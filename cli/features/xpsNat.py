#!/usr/bin/env python
#  xpsNat.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

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
# The class object for xpsNat operations
#/**********************************************************************************/

class xpsNatObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsNatInitScope
    #/********************************************************************************/
    def do_nat_init_scope(self, arg):
        '''
         xpsNatInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsNatInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsNatInit
    #/********************************************************************************/
    def do_nat_init(self, arg):
        '''
         xpsNatInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsNatInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNatDeInitScope
    #/********************************************************************************/
    def do_nat_de_init_scope(self, arg):
        '''
         xpsNatDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsNatDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsNatDeInit
    #/********************************************************************************/
    def do_nat_de_init(self, arg):
        '''
         xpsNatDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsNatDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for initType enumerations
    #/********************************************************************************/
    def complete_nat_add_device(self, text, line, begidx, endidx):
        tempDict = { 2 : 'initType'}
        return enumDict.autoCompletion(line, text, tempDict)    

    #/********************************************************************************/
    # command for xpsNatAddDevice
    #/********************************************************************************/
    def do_nat_add_device(self, arg):
        '''
         xpsNatAddDevice: Enter [ devId,initType ]
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
            ret = xpsNatAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNatRemoveDevice
    #/********************************************************************************/
    def do_nat_remove_device(self, arg):
        '''
         xpsNatRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsNatRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNatSetMdtConfig
    #/********************************************************************************/
    def do_nat_set_mdt_config(self, arg):
        '''
         xpsNatSetMdtConfig: Enter [ devId,index,value ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,value ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, index=%d, value=0x%x' % (args[0],args[1],args[2]))
            ret = xpsNatSetMdtConfig(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNatDeleteEntry
    #/********************************************************************************/
    def do_nat_delete_entry(self, arg):
        '''
         xpsNatDeleteEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsNatDeleteEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNatGetEntry
    #/********************************************************************************/
    def do_nat_get_entry(self, arg):
        '''
         xpsNatGetEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNatEntryKey_t_Ptr = new_xpsNatEntryKey_tp()
            xpsNatEntryMask_t_Ptr = new_xpsNatEntryMask_tp()
            xpsNatEntryData_t_Ptr = new_xpsNatEntryData_tp()
            
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsNatGetEntry(args[0],args[1],xpsNatEntryKey_t_Ptr,xpsNatEntryMask_t_Ptr,xpsNatEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                listLen = 4
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryKey_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryKey_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryKey_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryKey_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryKey_t_Ptr.Protocol))
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryMask_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryMask_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryMask_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryMask_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryMask_t_Ptr.Protocol))
                print('SIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.SIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('srcPort = %d' % (xpsNatEntryData_t_Ptr.srcPort))
                print('DIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.DIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('destPort = %d' % (xpsNatEntryData_t_Ptr.destPort))
                print('vif = %d' % (xpsNatEntryData_t_Ptr.vif))
                pass
            delete_xpsNatEntryData_tp(xpsNatEntryData_t_Ptr)
            delete_xpsNatEntryMask_tp(xpsNatEntryMask_t_Ptr)
            delete_xpsNatEntryKey_tp(xpsNatEntryKey_t_Ptr)

    #/********************************************************************************/
    # command for xpsNatAddInternalEntry
    #/********************************************************************************/

    def do_nat_add_internal_entry(self, arg):
        '''
          xpsNatAddInternalEntry: Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 21
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNatEntryKey_t_Ptr = new_xpsNatEntryKey_tp()
            
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.SrcPort = int(args[3])
            
            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.DestPort = int(args[5])
            xpsNatEntryKey_t_Ptr.Bd = int(args[6])
            xpsNatEntryKey_t_Ptr.Flag = int(args[7])
            xpsNatEntryKey_t_Ptr.Protocol = int(args[8])
            xpsNatEntryMask_t_Ptr = new_xpsNatEntryMask_tp()
            
            args[9] = args[9].replace(".",":").replace(",",":")
            postList = args[9].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryMask_t_Ptr.SrcPort = int(args[10])

            args[11] = args[11].replace(".",":").replace(",",":")
            postList = args[11].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])


            xpsNatEntryMask_t_Ptr.DestPort = int(args[12])
            xpsNatEntryMask_t_Ptr.Bd = int(args[13])
            xpsNatEntryMask_t_Ptr.Flag = int(args[14])
            xpsNatEntryMask_t_Ptr.Protocol = int(args[15])
 
            xpsNatEntryData_t_Ptr = new_xpsNatEntryData_tp()
     

            args[16] = args[16].replace(".",":").replace(",",":")
            postList = args[16].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.SIPAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryData_t_Ptr.srcPort = int(args[17])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.DIPAddress[listLen - ix - 1] = int(postList[ix])
 
            xpsNatEntryData_t_Ptr.destPort = int(args[19])
            xpsNatEntryData_t_Ptr.vif = int(args[20])

            #print('Input Arguments are, devId=%d, index=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SIPAddress=%s, srcPort=%d, DIPAddress=%s, destPort=%d, vif=%d' % (args[0],args[1],args[2],xpsNatEntryKey_t_Ptr.SrcPort,args[4],xpsNatEntryKey_t_Ptr.DestPort,xpsNatEntryKey_t_Ptr.Bd,xpsNatEntryKey_t_Ptr.Flag,xpsNatEntryKey_t_Ptr.Protocol,args[9],xpsNatEntryMask_t_Ptr.SrcPort,args[11],xpsNatEntryMask_t_Ptr.DestPort,xpsNatEntryMask_t_Ptr.Bd,xpsNatEntryMask_t_Ptr.Flag,xpsNatEntryMask_t_Ptr.Protocol,args[16],xpsNatEntryData_t_Ptr.srcPort,args[18],xpsNatEntryData_t_Ptr.destPort,xpsNatEntryData_t_Ptr.vif))
            ret = xpsNatAddInternalEntry(args[0],args[1],xpsNatEntryKey_t_Ptr,xpsNatEntryMask_t_Ptr,xpsNatEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryKey_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryKey_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryKey_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryKey_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryKey_t_Ptr.Protocol))
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryMask_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryMask_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryMask_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryMask_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryMask_t_Ptr.Protocol))
                print('SIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.SIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('srcPort = %d' % (xpsNatEntryData_t_Ptr.srcPort))
                print('DIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.DIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('destPort = %d' % (xpsNatEntryData_t_Ptr.destPort))
                print('vif = %d' % (xpsNatEntryData_t_Ptr.vif))
                pass
            delete_xpsNatEntryData_tp(xpsNatEntryData_t_Ptr)
            delete_xpsNatEntryMask_tp(xpsNatEntryMask_t_Ptr)
            delete_xpsNatEntryKey_tp(xpsNatEntryKey_t_Ptr)
    #/********************************************************************************/
    # command for xpsNatAddDoubleEntry
    #/********************************************************************************/
    def do_nat_add_double_entry(self, arg):
        '''
          xpsNatAddDoubleEntry: Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 21
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNatEntryKey_t_Ptr = new_xpsNatEntryKey_tp()
            
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.SrcPort = int(args[3])
            
            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.DestPort = int(args[5])
            xpsNatEntryKey_t_Ptr.Bd = int(args[6])
            xpsNatEntryKey_t_Ptr.Flag = int(args[7])
            xpsNatEntryKey_t_Ptr.Protocol = int(args[8])
            xpsNatEntryMask_t_Ptr = new_xpsNatEntryMask_tp()
            
            args[9] = args[9].replace(".",":").replace(",",":")
            postList = args[9].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryMask_t_Ptr.SrcPort = int(args[10])

            args[11] = args[11].replace(".",":").replace(",",":")
            postList = args[11].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])


            xpsNatEntryMask_t_Ptr.DestPort = int(args[12])
            xpsNatEntryMask_t_Ptr.Bd = int(args[13])
            xpsNatEntryMask_t_Ptr.Flag = int(args[14])
            xpsNatEntryMask_t_Ptr.Protocol = int(args[15])
 
            xpsNatEntryData_t_Ptr = new_xpsNatEntryData_tp()
     

            args[16] = args[16].replace(".",":").replace(",",":")
            postList = args[16].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.SIPAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryData_t_Ptr.srcPort = int(args[17])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.DIPAddress[listLen - ix - 1] = int(postList[ix])
 
            xpsNatEntryData_t_Ptr.destPort = int(args[19])
            xpsNatEntryData_t_Ptr.vif = int(args[20])

            #print('Input Arguments are, devId=%d, index=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SIPAddress=%s, srcPort=%d, DIPAddress=%s, destPort=%d, vif=%d' % (args[0],args[1],args[2],xpsNatEntryKey_t_Ptr.SrcPort,args[4],xpsNatEntryKey_t_Ptr.DestPort,xpsNatEntryKey_t_Ptr.Bd,xpsNatEntryKey_t_Ptr.Flag,xpsNatEntryKey_t_Ptr.Protocol,args[9],xpsNatEntryMask_t_Ptr.SrcPort,args[11],xpsNatEntryMask_t_Ptr.DestPort,xpsNatEntryMask_t_Ptr.Bd,xpsNatEntryMask_t_Ptr.Flag,xpsNatEntryMask_t_Ptr.Protocol,args[16],xpsNatEntryData_t_Ptr.srcPort,args[18],xpsNatEntryData_t_Ptr.destPort,xpsNatEntryData_t_Ptr.vif))
            ret = xpsNatAddDoubleEntry(args[0],args[1],xpsNatEntryKey_t_Ptr,xpsNatEntryMask_t_Ptr,xpsNatEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryKey_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryKey_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryKey_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryKey_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryKey_t_Ptr.Protocol))
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryMask_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryMask_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryMask_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryMask_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryMask_t_Ptr.Protocol))
                print('SIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.SIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('srcPort = %d' % (xpsNatEntryData_t_Ptr.srcPort))
                print('DIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.DIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('destPort = %d' % (xpsNatEntryData_t_Ptr.destPort))
                print('vif = %d' % (xpsNatEntryData_t_Ptr.vif))
                pass
            delete_xpsNatEntryData_tp(xpsNatEntryData_t_Ptr)
            delete_xpsNatEntryMask_tp(xpsNatEntryMask_t_Ptr)
            delete_xpsNatEntryKey_tp(xpsNatEntryKey_t_Ptr)
   #/********************************************************************************/
    # command for xpsNatAddExternalEntry
    #/********************************************************************************/

    def do_nat_add_external_entry(self, arg):
        '''
          xpsNatAddExternalEntry: Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 21
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNatEntryKey_t_Ptr = new_xpsNatEntryKey_tp()
            
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.SrcPort = int(args[3])
            
            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.DestPort = int(args[5])
            xpsNatEntryKey_t_Ptr.Bd = int(args[6])
            xpsNatEntryKey_t_Ptr.Flag = int(args[7])
            xpsNatEntryKey_t_Ptr.Protocol = int(args[8])
            xpsNatEntryMask_t_Ptr = new_xpsNatEntryMask_tp()
            
            args[9] = args[9].replace(".",":").replace(",",":")
            postList = args[9].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryMask_t_Ptr.SrcPort = int(args[10])

            args[11] = args[11].replace(".",":").replace(",",":")
            postList = args[11].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])


            xpsNatEntryMask_t_Ptr.DestPort = int(args[12])
            xpsNatEntryMask_t_Ptr.Bd = int(args[13])
            xpsNatEntryMask_t_Ptr.Flag = int(args[14])
            xpsNatEntryMask_t_Ptr.Protocol = int(args[15])
 
            xpsNatEntryData_t_Ptr = new_xpsNatEntryData_tp()
     

            args[16] = args[16].replace(".",":").replace(",",":")
            postList = args[16].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.SIPAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryData_t_Ptr.srcPort = int(args[17])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.DIPAddress[listLen - ix - 1] = int(postList[ix])
 
            xpsNatEntryData_t_Ptr.destPort = int(args[19])
            xpsNatEntryData_t_Ptr.vif = int(args[20])

            #print('Input Arguments are, devId=%d, index=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SIPAddress=%s, srcPort=%d, DIPAddress=%s, destPort=%d, vif=%d' % (args[0],args[1],args[2],xpsNatEntryKey_t_Ptr.SrcPort,args[4],xpsNatEntryKey_t_Ptr.DestPort,xpsNatEntryKey_t_Ptr.Bd,xpsNatEntryKey_t_Ptr.Flag,xpsNatEntryKey_t_Ptr.Protocol,args[9],xpsNatEntryMask_t_Ptr.SrcPort,args[11],xpsNatEntryMask_t_Ptr.DestPort,xpsNatEntryMask_t_Ptr.Bd,xpsNatEntryMask_t_Ptr.Flag,xpsNatEntryMask_t_Ptr.Protocol,args[16],xpsNatEntryData_t_Ptr.srcPort,args[18],xpsNatEntryData_t_Ptr.destPort,xpsNatEntryData_t_Ptr.vif))
            ret = xpsNatAddExternalEntry(args[0],args[1],xpsNatEntryKey_t_Ptr,xpsNatEntryMask_t_Ptr,xpsNatEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryKey_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryKey_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryKey_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryKey_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryKey_t_Ptr.Protocol))
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryMask_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryMask_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryMask_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryMask_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryMask_t_Ptr.Protocol))
                print('SIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.SIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('srcPort = %d' % (xpsNatEntryData_t_Ptr.srcPort))
                print('DIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.DIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('destPort = %d' % (xpsNatEntryData_t_Ptr.destPort))
                print('vif = %d' % (xpsNatEntryData_t_Ptr.vif))
                pass
            delete_xpsNatEntryData_tp(xpsNatEntryData_t_Ptr)
            delete_xpsNatEntryMask_tp(xpsNatEntryMask_t_Ptr)
            delete_xpsNatEntryKey_tp(xpsNatEntryKey_t_Ptr)

    #/********************************************************************************/
    # command for xpsNatAddFilterRule
    #/********************************************************************************/

    def do_nat_add_filter_rule(self, arg):
        '''
          xpsNatAddFilterRule: Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 21
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SrcAddress,SrcPort,DestAddress,DestPort,Bd,Flag,Protocol,SIPAddress,srcPort,DIPAddress,destPort,vif ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNatEntryKey_t_Ptr = new_xpsNatEntryKey_tp()
            
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.SrcPort = int(args[3])
            
            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryKey_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])
            xpsNatEntryKey_t_Ptr.DestPort = int(args[5])
            xpsNatEntryKey_t_Ptr.Bd = int(args[6])
            xpsNatEntryKey_t_Ptr.Flag = int(args[7])
            xpsNatEntryKey_t_Ptr.Protocol = int(args[8])
            xpsNatEntryMask_t_Ptr = new_xpsNatEntryMask_tp()
            
            args[9] = args[9].replace(".",":").replace(",",":")
            postList = args[9].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.SrcAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryMask_t_Ptr.SrcPort = int(args[10])

            args[11] = args[11].replace(".",":").replace(",",":")
            postList = args[11].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryMask_t_Ptr.DestAddress[listLen - ix - 1] = int(postList[ix])


            xpsNatEntryMask_t_Ptr.DestPort = int(args[12])
            xpsNatEntryMask_t_Ptr.Bd = int(args[13])
            xpsNatEntryMask_t_Ptr.Flag = int(args[14])
            xpsNatEntryMask_t_Ptr.Protocol = int(args[15])
 
            xpsNatEntryData_t_Ptr = new_xpsNatEntryData_tp()
     

            args[16] = args[16].replace(".",":").replace(",",":")
            postList = args[16].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.SIPAddress[listLen - ix - 1] = int(postList[ix])

            xpsNatEntryData_t_Ptr.srcPort = int(args[17])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsNatEntryData_t_Ptr.DIPAddress[listLen - ix - 1] = int(postList[ix])
 
            xpsNatEntryData_t_Ptr.destPort = int(args[19])
            xpsNatEntryData_t_Ptr.vif = int(args[20])

            #print('Input Arguments are, devId=%d, index=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SrcAddress=%s, SrcPort=%d, DestAddress=%s, DestPort=%d, Bd=%d, Flag=%d, Protocol=%d, SIPAddress=%s, srcPort=%d, DIPAddress=%s, destPort=%d, vif=%d' % (args[0],args[1],args[2],xpsNatEntryKey_t_Ptr.SrcPort,args[4],xpsNatEntryKey_t_Ptr.DestPort,xpsNatEntryKey_t_Ptr.Bd,xpsNatEntryKey_t_Ptr.Flag,xpsNatEntryKey_t_Ptr.Protocol,args[9],xpsNatEntryMask_t_Ptr.SrcPort,args[11],xpsNatEntryMask_t_Ptr.DestPort,xpsNatEntryMask_t_Ptr.Bd,xpsNatEntryMask_t_Ptr.Flag,xpsNatEntryMask_t_Ptr.Protocol,args[16],xpsNatEntryData_t_Ptr.srcPort,args[18],xpsNatEntryData_t_Ptr.destPort,xpsNatEntryData_t_Ptr.vif))
            ret = xpsNatAddFilterRule(args[0],args[1],xpsNatEntryKey_t_Ptr,xpsNatEntryMask_t_Ptr,xpsNatEntryData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryKey_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryKey_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryKey_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryKey_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryKey_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryKey_t_Ptr.Protocol))
                print('SrcAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.SrcAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('SrcPort = %d' % (xpsNatEntryMask_t_Ptr.SrcPort))
                print('DestAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryMask_t_Ptr.DestAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('DestPort = %d' % (xpsNatEntryMask_t_Ptr.DestPort))
                print('Bd = %d' % (xpsNatEntryMask_t_Ptr.Bd))
                print('Flag = %d' % (xpsNatEntryMask_t_Ptr.Flag))
                print('Protocol = %d' % (xpsNatEntryMask_t_Ptr.Protocol))
                print('SIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.SIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('srcPort = %d' % (xpsNatEntryData_t_Ptr.srcPort))
                print('DIPAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsNatEntryData_t_Ptr.DIPAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('destPort = %d' % (xpsNatEntryData_t_Ptr.destPort))
                print('vif = %d' % (xpsNatEntryData_t_Ptr.vif))
                pass
            delete_xpsNatEntryData_tp(xpsNatEntryData_t_Ptr)
            delete_xpsNatEntryMask_tp(xpsNatEntryMask_t_Ptr)
            delete_xpsNatEntryKey_tp(xpsNatEntryKey_t_Ptr)


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


#!/usr/bin/env python
#  xpsAcm.py
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
# The class object for xpsAcm operations
#/**********************************************************************************/

class xpsAcmObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsAcmInit
    #/********************************************************************************/
    def do_acm_init(self, arg):
        '''
         xpsAcmInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsAcmInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmInitScope
    #/********************************************************************************/
    def do_acm_init_scope(self, arg):
        '''
         xpsAcmInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsAcmInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmDeInit
    #/********************************************************************************/
    def do_acm_de_init(self, arg):
        '''
         xpsAcmDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsAcmDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmDeInitScope
    #/********************************************************************************/
    def do_acm_de_init_scope(self, arg):
        '''
         xpsAcmDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsAcmDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmAddDevice
    #/********************************************************************************/
    def do_acm_add_device(self, arg):
        '''
         xpsAcmAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsAcmAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmRemoveDevice
    #/********************************************************************************/
    def do_acm_remove_device(self, arg):
        '''
         xpsAcmRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsAcmRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_get_counter_value(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmGetCounterValue
    #/********************************************************************************/
    def do_acm_get_counter_value(self, arg):
        '''
         xpsAcmGetCounterValue: Enter [ devId,client,countIndex ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,countIndex ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            fData_Ptr_3 = new_uint64_tp()
            fData_Ptr_4 = new_uint64_tp()
            #print('Input Arguments are, devId=%d, client=%d, countIndex=%d' % (args[0],args[1],args[2]))
            ret = xpsAcmGetCounterValue(args[0],args[1],args[2],fData_Ptr_3,fData_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Num Pkts = %ld, Num Bytes = %ld' % (uint64_tp_value(fData_Ptr_3),uint64_tp_value(fData_Ptr_4)))
                pass
            delete_uint64_tp(fData_Ptr_3)
            delete_uint64_tp(fData_Ptr_4)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_print_counter_value(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmPrintCounterValue
    #/********************************************************************************/
    def do_acm_print_counter_value(self, arg):
        '''
         xpsAcmPrintCounterValue: Enter [ devId,client,countIndex,printZeros ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,countIndex,printZeros ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, countIndex=%d, printZeros=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsAcmPrintCounterValue(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %ld' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_print_all_counter_value(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmPrintAllCounterValue
    #/********************************************************************************/
    def do_acm_print_all_counter_value(self, arg):
        '''
         xpsAcmPrintAllCounterValue: Enter [ devId,client,printZeros ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,printZeros ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, client=%d, printZeros=0x%x' % (args[0],args[1],args[2]))
            ret = xpsAcmPrintAllCounterValue(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_get_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmGetSamplingConfig
    #/********************************************************************************/
    def do_acm_get_sampling_config(self, arg):
        '''
         xpsAcmGetSamplingConfig: Enter [ devId,client,index ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            nSample_Ptr_3 = new_uint32_tp()
            mBase_Ptr_4 = new_uint32_tp()
            mExpo_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsAcmGetSamplingConfig(args[0],args[1],args[2],nSample_Ptr_3,mBase_Ptr_4,mExpo_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nSample = %d' % (uint32_tp_value(nSample_Ptr_3)))
                print('mBase = %d' % (uint32_tp_value(mBase_Ptr_4)))
                print('mExpo = %d' % (uint32_tp_value(mExpo_Ptr_5)))
                pass
            delete_uint32_tp(mExpo_Ptr_5)
            delete_uint32_tp(mBase_Ptr_4)
            delete_uint32_tp(nSample_Ptr_3)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_set_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmSetSamplingConfig
    #/********************************************************************************/
    def do_acm_set_sampling_config(self, arg):
        '''
         xpsAcmSetSamplingConfig: Enter [ devId,client,index,nSample,mBase,mExpo ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,nSample,mBase,mExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, nSample=%d, mBase=%d, mExpo=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsAcmSetSamplingConfig(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_get_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmGetSamplingState
    #/********************************************************************************/
    def do_acm_get_sampling_state(self, arg):
        '''
         xpsAcmGetSamplingState: Enter [ devId,client,index ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            totalCnt_Ptr_3 = new_uint64_tp()
            interEventCnt_Ptr_4 = new_uint32_tp()
            interSampleStart_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, client=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsAcmGetSamplingState(args[0],args[1],args[2],totalCnt_Ptr_3,interEventCnt_Ptr_4,interSampleStart_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('totalCnt = %ld' % (uint64_tp_value(totalCnt_Ptr_3)))
                print('interEventCnt = %d' % (uint32_tp_value(interEventCnt_Ptr_4)))
                print('interSampleStart = %d' % (uint32_tp_value(interSampleStart_Ptr_5)))
                pass
            delete_uint32_tp(interSampleStart_Ptr_5)
            delete_uint32_tp(interEventCnt_Ptr_4)
            delete_uint64_tp(totalCnt_Ptr_3)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_set_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmSetSamplingState
    #/********************************************************************************/
    def do_acm_set_sampling_state(self, arg):
        '''
         xpsAcmSetSamplingState: Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,index,totalCnt,interEventCnt,interSampleStart ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, client=%d, index=%d, totalCnt=0x%x, interEventCnt=%d, interSampleStart=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsAcmSetSamplingState(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmCntSetGlobalConfigBucketization
    #/********************************************************************************/
    def do_acm_cnt_set_global_config_bucketization(self, arg):
        '''
         xpsAcmCntSetGlobalConfigBucketization: Enter [ devId,enable,startRange,endRange,numBkts,granularity,addAddr,bktUseAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,startRange,endRange,numBkts,granularity,addAddr,bktUseAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6], 16)
            args[7] = int(args[7], 16)
            #print('Input Arguments are, devId=%d, enable=%d, startRange=0x%x, endRange=0x%x, numBkts=%d, granularity=%d, addAddr=0x%x, bktUseAddr=0x%x' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]))
            ret = xpsAcmCntSetGlobalConfigBucketization(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmCntSetGlobalConfigModePol
    #/********************************************************************************/
    def do_acm_cnt_set_global_config_mode_pol(self, arg):
        '''
         xpsAcmCntSetGlobalConfigModePol: Enter [ devId,refreshEnable,unitTime,refrTimeGranularity,updateWeight,billingCntrEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,refreshEnable,unitTime,refrTimeGranularity,updateWeight,billingCntrEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, refreshEnable=%d, unitTime=%d, refrTimeGranularity=%d, updateWeight=%d, billingCntrEnable=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsAcmCntSetGlobalConfigModePol(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_print_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmPrintSamplingConfig
    #/********************************************************************************/
    def do_acm_print_sampling_config(self, arg):
        '''
         xpsAcmPrintSamplingConfig: Enter [ devId,client,index ]
		 Valid values for client : <acmClient>
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
            ret = xpsAcmPrintSamplingConfig(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_acm_print_sampling_state(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAcmPrintSamplingState
    #/********************************************************************************/
    def do_acm_print_sampling_state(self, arg):
        '''
         xpsAcmPrintSamplingState: Enter [ devId,client,index ]
		 Valid values for client : <acmClient>
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
            ret = xpsAcmPrintSamplingState(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_allocate_acm(self, text, line, begidx, endidx):
        tempDict = { 1 : 'acmClient'}
        return enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsAllocateAcm
    #/********************************************************************************/
    def do_allocate_acm(self, arg):
        '''
         xpsAllocateAcm: Enter [ client,type ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ client,type ]')
        else:
            args[0] = eval(args[0])
            args[1] = int(args[1])
            acmId_Ptr_2 = new_xpAcm_tp()
            #print('Input Arguments are, client=%d, type=%d' % (args[0],args[1]))
            ret = xpsAllocateAcm(args[0],args[1],acmId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('acmId = %d' % (xpAcm_tp_value(acmId_Ptr_2)))
                pass
            delete_xpAcm_tp(acmId_Ptr_2)

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_set_count_mode(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xpsSetCountMode
    #/********************************************************************************/
    def do_set_count_mode(self, arg):
        '''
         xpsSetCountMode: Enter [ devId,client,type,bankMode,clearOnRead,wrapAround,countOffset ]
		 Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type,bankMode,clearOnRead,wrapAround,countOffset ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            #print('Input Arguments are, devId=%d, client=%d, type=%d, bankMode=%d, clearOnRead=%d, wrapAround=%d, countOffset=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6]))
            ret = xpsSetCountMode(args[0],args[1],args[2],args[3],args[4],args[5],args[6])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmSetLaneIndexBaseSel
    #/********************************************************************************/
    def do_acm_set_lane_index_base_sel(self, arg):
        '''
         xpsAcmSetLaneIndexBaseSel: Enter [ devId,client,type,baseSel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type,baseSel ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, client=%d, type=%d, baseSel=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsAcmSetLaneIndexBaseSel(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcmGetLaneIndexBaseSel
    #/********************************************************************************/
    def do_acm_get_lane_index_base_sel(self, arg):
        '''
         xpsAcmGetLaneIndexBaseSel: Enter [ devId,client,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,type ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            baseSel_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, client=%d, type=%d' % (args[0],args[1],args[2]))
            ret = xpsAcmGetLaneIndexBaseSel(args[0],args[1],args[2],baseSel_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('baseSel = %d' % (uint32_tp_value(baseSel_Ptr_3)))
                pass
            delete_uint32_tp(baseSel_Ptr_3)
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


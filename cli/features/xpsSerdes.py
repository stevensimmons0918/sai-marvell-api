#!/usr/bin/env python
#  xpsSerdes.py
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
import inspect
from multiprocessing import Process

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

def threaded_function(*args):
    ret = xpsSerdesAacsServer(int(args[0]), int(args[1]), int(args[2]))
    if ret < 0:
        print 'serdes_aacs_server Failed with return value:', ret
    else:
        print 'done with return value:', ret

#/**********************************************************************************/
# Prepare serdes eye configuration object
#/**********************************************************************************/

def customPrepareEyeCfgData(args):
    varArgDict = {
                   "-eyeType" : "0",
                   "-cmpMode" : "0",
                   "-minDwellBits" : "0",
                   "-maxDwellBits" : "0",
                   "-fastDynamic" : "0",
                   "-errorThreshold" : "0",
                   "-dcBalance" : "0",
                   "-transDensity" : "0",
                   "-xUI" : "0",
                   "-xResolution" : "0",
                   "-yCenterPoint" : "0",
                   "-yPoints" : "0",
                   "-yStepSize" : "0",
                   "-xAutoScale" : "0",
                   "-yAutoScale" : "0",
                   "-xShift" : "0",
                   "-gatherMean" : "0",
                   "-meanThreshold" : "0",
                   "-noSbm" : "0",
                   "-meanDirection" : "0",
           }
    #default eye config values
    eyecfg = new_xpSerdesEyeConfig_tp()
    retVal = xpsSerdesGetEyeWithDefaultConfig (int(args[0]), int(args[1]), eyecfg);
    if(retVal != 0):
        print 'Error in getting default eye config, Return Value : %d' %(retVal)
        return None

    for index, argument in enumerate(args):
    	if "-" in argument :
    		if varArgDict.has_key(argument):
    			if (index + 1) < len(args):
    				varArgDict[argument] = args[index + 1]
                                if "-eyeType" in argument:
                                    eyecfg.ecEyeType = int(varArgDict["-eyeType"])
                                if "-cmpMode" in argument:
                                    eyecfg.ecCmpMode = int(varArgDict["-cmpMode"], 16)
                                if "-minDwellBits" in argument:
                                    eyecfg.ecMinDwellBits = long(varArgDict["-minDwellBits"])
                                if "-maxDwellBits" in argument:
                                    eyecfg.ecMaxDwellBits = long(varArgDict["-maxDwellBits"])
                                if "-fastDynamic" in argument:
                                    eyecfg.ecFastDynamic = int(varArgDict["-fastDynamic"])
                                if "-errorThreshold" in argument:
                                    eyecfg.ecErrorThreshold = int(varArgDict["-errorThreshold"])
                                if "-dcBalance" in argument:
                                    eyecfg.ecDcBalance = float(varArgDict["-dcBalance"])
                                if "-transDensity" in argument:
                                    eyecfg.ecTransDensity = float(varArgDict["-transDensity"])
                                if "-xUI" in argument:
                                    eyecfg.ecXUI = int(varArgDict["-xUI"])
                                if "-xResolution" in argument:
                                    eyecfg.ecXResolution = int(varArgDict["-xResolution"])
                                if "-yCenterPoint" in argument:
                                    eyecfg.ecYCenterPoint = int(varArgDict["-yCenterPoint"])
                                if "-yPoints" in argument:
                                    eyecfg.ecYPoints = int(varArgDict["-yPoints"])
                                if "-yStepSize" in argument:
                                    eyecfg.ecYStepSize = int(varArgDict["-yStepSize"])
                                if "-xAutoScale" in argument:
                                    eyecfg.ecXAutoScale = int(varArgDict["-xAutoScale"])
                                if "-yAutoScale" in argument:
                                    eyecfg.ecYAutoScale = int(varArgDict["-yAutoScale"])
                                if "-xShift" in argument:
                                    eyecfg.ecXShift = int(varArgDict["-xShift"])
                                if "-gatherMean" in argument:
                                    eyecfg.ecGatherMean = bool(varArgDict["-gatherMean"])
                                if "-meanThreshold" in argument:
                                    eyecfg.ecMeanThreshold = long(varArgDict["-meanThreshold"])
                                if "-noSbm" in argument:
                                    eyecfg.ecNoSbm = bool(varArgDict["-noSbm"])
                                if "-meanDirection" in argument:
                                    eyecfg.ecMeanDirection = int(varArgDict["-meanDirection"])
    			else:
    				print "data missing in argument : %s"%(argument)
    				return None
    		else:
    			print  "invalid argument : %s"%(argument)
    			return None
    print ('eyecfg.ec_eye_type        = %d' % eyecfg.ecEyeType)
    print ('eyecfg.ec_cmp_mode        = %d' % eyecfg.ecCmpMode)
    print ('eyecfg.ec_max_dwell_bits  = %i' % eyecfg.ecMaxDwellBits)
    print ('eyecfg.ec_min_dwell_bits  = %i' % eyecfg.ecMinDwellBits)
    print ('eyecfg.ec_fast_dynamic    = %d' % eyecfg.ecFastDynamic)
    print ('eyecfg.ec_error_threshold = %d' % eyecfg.ecErrorThreshold )
    print ('eyecfg.ec_x_UI            = %d' % eyecfg.ecXUI)
    print ('eyecfg.ec_x_resolution    = %d' % eyecfg.ecXResolution)
    print ('eyecfg.ec_y_center_point  = %d' % eyecfg.ecYCenterPoint)
    print ('eyecfg.ec_y_step_size     = %d' % eyecfg.ecYStepSize)
    print ('eyecfg.ec_y_points        = %d' % eyecfg.ecYPoints)
    print ('eyecfg.ec_dc_balance      = %f' % eyecfg.ecDcBalance)
    print ('eyecfg.ec_trans_density   = %f' % eyecfg.ecTransDensity)
    print ('eyecfg.ec_x_auto_scale    = %d' % eyecfg.ecXAutoScale)
    print ('eyecfg.ec_y_auto_scale    = %d' % eyecfg.ecYAutoScale)
    print ('eyecfg.ec_x_shift         = %d' % eyecfg.ecXShift )
    print ('eyecfg.ec_gather_mean     = %d' % eyecfg.ecGatherMean)
    print ('eyecfg.ec_mean_threshold  = %i' % eyecfg.ecMeanThreshold)
    print ('eyecfg.ec_no_sbm          = %d' % eyecfg.ecNoSbm)
    print ('eyecfg.ec_mean_direction  = %d' % eyecfg.ecMeanDirection )
    print ('----------------------------------')

    return eyecfg



#/**********************************************************************************/
# The class object for xpsSerdes operations
#/**********************************************************************************/

class xpsSerdesObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # command to serdes aacs server
    #/*****************************************************************************************************/
    def do_serdes_aacs_server(self, arg):
        'Run serdes_aacs_server command as different process, Enter serdes_aacs_server command with arguments'
        '''
         Run serdes_aacs_server: Enter [ devId,serdesId,tcpPort ]
        '''
        args =  arg.split()
        if  len(args) < 3:
            print('Invalid input, Enter [ devId, serdesId, tcpPort ]')
        else:
            print('Input Arguments are, devId=%d serdesId=%d tcpPort=%d' % (int(args[0]), int(args[1]), int(args[2])))
            p1 = Process(target = threaded_function, args = tuple(args))
            p1.start()

    #/********************************************************************************/
    # command for xpsSerdesAaplGetReturnCode
    #/********************************************************************************/
    def do_serdes_aapl_get_return_code(self, arg):
        '''
         xpsSerdesAaplGetReturnCode: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesAaplGetReturnCode(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            pass

    #/********************************************************************************/
    # command for xpsSerdesAaplSetIpType
    #/********************************************************************************/
    def do_serdes_aapl_set_ip_type(self, arg):
        '''
         xpsSerdesAaplSetIpType: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesAaplSetIpType(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesHardwareInfoFormat
    #/********************************************************************************/
    def do_serdes_hardware_info_format(self, arg):
        '''
         xpsSerdesHardwareInfoFormat: Enter [ devId,serdesId,hwInfo ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,hwInfo ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            hwInfo_ptr = str(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, hwInfo=%s' % (args[0],args[1],hwInfo_ptr))
            ret = xpsSerdesHardwareInfoFormat(args[0],args[1],hwInfo_ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesDiagSerdesRwTest
    #/********************************************************************************/
    def do_serdes_diag_serdes_rw_test(self, arg):
        '''
         xpsSerdesDiagSerdesRwTest: Enter [ devId,serdesId,cycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,cycles ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, cycles=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesDiagSerdesRwTest(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesDiagSerdesDump
    #/********************************************************************************/
    def do_serdes_diag_serdes_dump(self, arg):
        '''
         xpsSerdesDiagSerdesDump: Enter [ devId,serdesId,binEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,binEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, binEnable=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesDiagSerdesDump(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesDiag1
    #/********************************************************************************/
    def do_serdes_diag1(self, arg):
        '''
         xpsSerdesDiag1: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesDiag1(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetAaplLog
    #/********************************************************************************/
    def do_serdes_get_aapl_log(self, arg):
        '''
         xpsSerdesGetAaplLog: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetAaplLog(args[0],args[1])

    #/********************************************************************************/
    # command for xpsSerdesDfePause
    #/********************************************************************************/
    def do_serdes_dfe_pause(self, arg):
        '''
         xpsSerdesDfePause: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rrEnabled_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesDfePause(args[0],args[1],rrEnabled_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rrEnabled = %d' % (uint32_tp_value(rrEnabled_Ptr_2)))
                pass
            delete_uint32_tp(rrEnabled_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesDfeWait
    #/********************************************************************************/
    def do_serdes_dfe_wait(self, arg):
        '''
         xpsSerdesDfeWait: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesDfeWait(args[0],args[1])
            if ret >= 0:
                print('return_value = %d ' % (ret))
                pass
            else:
                print('Error returned = %d' % (ret))

    #/********************************************************************************/
    # command for xpLinkManagerSerdesDfeWaitTimeout
    #/********************************************************************************/
    def do_serdes_dfe_wait_timeout(self, arg):
        '''
         xpsSerdesDfeWaitTimeout: Enter [ devId,serdesId/startSerdesId-endSerdesId,timeoutInMilliseconds ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,timeoutInMilliseconds ]')
        else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d timeoutInMilliseconds=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesDfeWaitTimeout(args[0],serdesId,args[2])
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass

    #/********************************************************************************/
    # command for xpsSerdesDfeResume
    #/********************************************************************************/
    def do_serdes_dfe_resume(self, arg):
        '''
         xpsSerdesDfeResume: Enter [ devId,serdesId,rrEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,rrEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, rrEnable=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesDfeResume(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesPmdTrain
    #/********************************************************************************/
    def do_serdes_pmd_train(self, arg):
        '''
         xpsSerdesPmdTrain: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesPmdTrain(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesPmdDebug
    #/********************************************************************************/
    def do_serdes_pmd_debug(self, arg):
        '''
         xpsSerdesPmdDebug: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesPmdDebug(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesPmdDebugPrint
    #/********************************************************************************/
    def do_serdes_pmd_debug_print(self, arg):
        '''
         xpsSerdesPmdDebugPrint: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesPmdDebugPrint(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesCommandExec
    #/********************************************************************************/
    def do_serdes_command_exec(self, arg):
        '''
         xpsSerdesCommandExec: Enter [ devId,serdesId,regAddr,command,serdesData,recvDataBack ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,regAddr,command,serdesData,recvDataBack ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3])
            args[4] = int(args[4], 16)
            args[5] = int(args[5], 16)
            serdesStatus_Ptr_6 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, regAddr=0x%x, command=%d, serdesData=0x%x, recvDataBack=0x%x' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsSerdesCommandExec(args[0],args[1],args[2],args[3],args[4],args[5],serdesStatus_Ptr_6)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_6)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_6)

    #/********************************************************************************/
    # command for xpsSerdesWr
    #/********************************************************************************/
    def do_serdes_wr(self, arg):
        '''
         xpsSerdesWr: Enter [ devId,serdesId,regAddr,serdesData ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,regAddr,serdesData ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            serdesStatus_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, regAddr=0x%x, serdesData=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesWr(args[0],args[1],args[2],args[3],serdesStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_4)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesRmw
    #/********************************************************************************/
    def do_serdes_rmw(self, arg):
        '''
         xpsSerdesRmw: Enter [ devId,serdesId,regAddr,serdesData,mask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,regAddr,serdesData,mask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            args[3] = int(args[3], 16)
            args[4] = int(args[4])
            initialVal_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, regAddr=0x%x, serdesData=0x%x, mask=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsSerdesRmw(args[0],args[1],args[2],args[3],args[4],initialVal_Ptr_5)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('initialVal = %d' % (uint32_tp_value(initialVal_Ptr_5)))
                pass
            delete_uint32_tp(initialVal_Ptr_5)

    #/********************************************************************************/
    # command for xpsSerdesRd
    #/********************************************************************************/
    def do_serdes_rd(self, arg):
        '''
         xpsSerdesRd: Enter [ devId,serdesId,regAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,regAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            rdData_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, regAddr=0x%x' % (args[0],args[1],args[2]))
            ret = xpsSerdesRd(args[0],args[1],args[2],rdData_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rdData = %d' % (uint32_tp_value(rdData_Ptr_3)))
                pass
            delete_uint32_tp(rdData_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesReset
    #/********************************************************************************/
    def do_serdes_reset(self, arg):
        '''
         xpsSerdesReset: Enter [ devId,serdesId,hard ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,hard ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, hard=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesReset(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsSerdesDiag
    #/********************************************************************************/
    def do_serdes_diag(self, arg):
        '''
         xpsSerdesDiag: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            serdesStatus_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesDiag(args[0],args[1],serdesStatus_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_2)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesStateDump
    #/********************************************************************************/
    def do_serdes_state_dump(self, arg):
        '''
         xpsSerdesStateDump: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesStateDump(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesEyeErrorsGet
    #/********************************************************************************/
    def do_serdes_eye_errors_get(self, arg):
        '''
         xpsSerdesEyeErrorsGet: Enter [ devId,serdesId,xPoint,yPoint ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,xPoint,yPoint ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            errCnt_Ptr_4 = new_int64_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, xPoint=%d, yPoint=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesEyeErrorsGet(args[0],args[1],args[2],args[3],errCnt_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('errCnt = %d' % (int64_tp_value(errCnt_Ptr_4)))
                pass
            delete_int64_tp(errCnt_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesEyeBitsGet
    #/********************************************************************************/
    def do_serdes_eye_bits_get(self, arg):
        '''
         xpsSerdesEyeBitsGet: Enter [ devId,serdesId,xPoint,yPoint ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,xPoint,yPoint ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            bitCnt_Ptr_4 = new_int64_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, xPoint=%d, yPoint=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesEyeBitsGet(args[0],args[1],args[2],args[3],bitCnt_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('bitCnt = %d' % (int64_tp_value(bitCnt_Ptr_4)))
                pass
            delete_int64_tp(bitCnt_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesEyeGradGet
    #/********************************************************************************/
    def do_serdes_eye_grad_get(self, arg):
        '''
         xpsSerdesEyeGradGet: Enter [ devId,serdesId,xPoint,yPoint ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,xPoint,yPoint ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            gradVal_Ptr_4 = new_Floatp()
            #print('Input Arguments are, devId=%d, serdesId=%d, xPoint=%d, yPoint=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesEyeGradGet(args[0],args[1],args[2],args[3],gradVal_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('gradVal = %d' % (Floatp_value(gradVal_Ptr_4)))
                pass
            delete_Floatp(gradVal_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesEyeQvalGet
    #/********************************************************************************/
    def do_serdes_eye_qval_get(self, arg):
        '''
         xpsSerdesEyeQvalGet: Enter [ devId,serdesId,xPoint,yPoint ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,xPoint,yPoint ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            qVal_Ptr_4 = new_Floatp()
            #print('Input Arguments are, devId=%d, serdesId=%d, xPoint=%d, yPoint=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesEyeQvalGet(args[0],args[1],args[2],args[3],qVal_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('qVal = %d' % (Floatp_value(qVal_Ptr_4)))
                pass
            delete_Floatp(qVal_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesEyePlotLogPrint
    #/********************************************************************************/
    def do_serdes_eye_plot_log_print(self, arg):
        '''
         xpsSerdesEyePlotLogPrint: Enter [ devId,serdesId,level,func,line ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,level,func,line ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            func_Ptr_3 = str(args[3])
            args[4] = int(args[4])

            #print('Input Arguments are, devId=%d, serdesId=%d, level=%d, func=%s, line=%d' % (args[0],args[1],args[2],func_Ptr_3,args[4]))
            ret = xpsSerdesEyePlotLogPrint(args[0],args[1],args[2],func_Ptr_3,args[4])
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('func = %s' % (func_Ptr_3))
                pass

    #/********************************************************************************/
    # command for xpsSerdesEyeHbtcLogPrint
    #/********************************************************************************/
    def do_serdes_eye_hbtc_log_print(self, arg):
        '''
         xpsSerdesEyeHbtcLogPrint: Enter [ devId,serdesId,level,func,line,leftIndex,rightIndex,leftPoints,rightPoints,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 31
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,level,func,line,leftIndex,rightIndex,leftPoints,rightPoints,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            func_Ptr_3 = str(args[3])
            args[4] = int(args[4])
            xpSerdesEyeHbtc_t_Ptr = new_xpSerdesEyeHbtc_tp()
            xpSerdesEyeHbtc_t_Ptr.leftIndex = int(args[5])
            xpSerdesEyeHbtc_t_Ptr.rightIndex = int(args[6])
            xpSerdesEyeHbtc_t_Ptr.leftPoints = int(args[7])
            xpSerdesEyeHbtc_t_Ptr.rightPoints = int(args[8])
            xpSerdesEyeHbtc_t_Ptr.points = int(args[9])
            xpSerdesEyeHbtc_t_Ptr.dataRow = int(args[10], 16)
            xpSerdesEyeHbtc_t_Ptr.totalRows = int(args[11])
            xpSerdesEyeHbtc_t_Ptr.transDensity = int(args[12])
            xpSerdesEyeHbtc_t_Ptr.dj = int(args[13])
            xpSerdesEyeHbtc_t_Ptr.leftRj = int(args[14])
            xpSerdesEyeHbtc_t_Ptr.leftSlope = int(args[15])
            xpSerdesEyeHbtc_t_Ptr.leftIntercept = int(args[16])
            xpSerdesEyeHbtc_t_Ptr.leftRSquared = int(args[17])
            xpSerdesEyeHbtc_t_Ptr.rightRj = int(args[18])
            xpSerdesEyeHbtc_t_Ptr.rightSlope = int(args[19])
            xpSerdesEyeHbtc_t_Ptr.rightIntercept = int(args[20])
            xpSerdesEyeHbtc_t_Ptr.rightRSquared = int(args[21])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e06 = int(args[22])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e10 = int(args[23])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e12 = int(args[24])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e15 = int(args[25])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e17 = int(args[26])
            xpSerdesEyeHbtc_t_Ptr.width0mUI = int(args[27])
            xpSerdesEyeHbtc_t_Ptr.width100mUI = int(args[28])
            xpSerdesEyeHbtc_t_Ptr.horzBer0mUI = int(args[29])
            xpSerdesEyeHbtc_t_Ptr.horzBer100mUI = int(args[30])
            #print('Input Arguments are, devId=%d, serdesId=%d, level=%d, func=%s, line=%d, leftIndex=%d, rightIndex=%d, leftPoints=%d, rightPoints=%d, points=%d, dataRow=0x%x, totalRows=%d, transDensity=%d, dj=%d, leftRj=%d, leftSlope=%d, leftIntercept=%d, leftRSquared=%d, rightRj=%d, rightSlope=%d, rightIntercept=%d, rightRSquared=%d, horzEye1e06=%d, horzEye1e10=%d, horzEye1e12=%d, horzEye1e15=%d, horzEye1e17=%d, width0mUI=%d, width100mUI=%d, horzBer0mUI=%d, horzBer100mUI=%d' % (args[0],args[1],args[2],func_Ptr_3,args[4],xpSerdesEyeHbtc_t_Ptr.leftIndex,xpSerdesEyeHbtc_t_Ptr.rightIndex,xpSerdesEyeHbtc_t_Ptr.leftPoints,xpSerdesEyeHbtc_t_Ptr.rightPoints,xpSerdesEyeHbtc_t_Ptr.points,xpSerdesEyeHbtc_t_Ptr.dataRow,xpSerdesEyeHbtc_t_Ptr.totalRows,xpSerdesEyeHbtc_t_Ptr.transDensity,xpSerdesEyeHbtc_t_Ptr.dj,xpSerdesEyeHbtc_t_Ptr.leftRj,xpSerdesEyeHbtc_t_Ptr.leftSlope,xpSerdesEyeHbtc_t_Ptr.leftIntercept,xpSerdesEyeHbtc_t_Ptr.leftRSquared,xpSerdesEyeHbtc_t_Ptr.rightRj,xpSerdesEyeHbtc_t_Ptr.rightSlope,xpSerdesEyeHbtc_t_Ptr.rightIntercept,xpSerdesEyeHbtc_t_Ptr.rightRSquared,xpSerdesEyeHbtc_t_Ptr.horzEye1e06,xpSerdesEyeHbtc_t_Ptr.horzEye1e10,xpSerdesEyeHbtc_t_Ptr.horzEye1e12,xpSerdesEyeHbtc_t_Ptr.horzEye1e15,xpSerdesEyeHbtc_t_Ptr.horzEye1e17,xpSerdesEyeHbtc_t_Ptr.width0mUI,xpSerdesEyeHbtc_t_Ptr.width100mUI,xpSerdesEyeHbtc_t_Ptr.horzBer0mUI,xpSerdesEyeHbtc_t_Ptr.horzBer100mUI))
            ret = xpsSerdesEyeHbtcLogPrint(args[0],args[1],args[2],func_Ptr_3,args[4],xpSerdesEyeHbtc_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('func = %s' % func_Ptr_3)
                print('leftIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIndex))
                print('rightIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIndex))
                print('leftPoints = %d' % (xpSerdesEyeHbtc_t_Ptr.leftPoints))
                print('rightPoints = %d' % (xpSerdesEyeHbtc_t_Ptr.rightPoints))
                print('points = %d' % (xpSerdesEyeHbtc_t_Ptr.points))
                print('dataRow = %d' % (xpSerdesEyeHbtc_t_Ptr.dataRow))
                print('totalRows = %d' % (xpSerdesEyeHbtc_t_Ptr.totalRows))
                print('transDensity = %d' % (xpSerdesEyeHbtc_t_Ptr.transDensity))
                print('dj = %d' % (xpSerdesEyeHbtc_t_Ptr.dj))
                print('leftRj = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRj))
                print('leftSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.leftSlope))
                print('leftIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIntercept))
                print('leftRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRSquared))
                print('rightRj = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRj))
                print('rightSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.rightSlope))
                print('rightIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIntercept))
                print('rightRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRSquared))
                print('horzEye1e06 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e06))
                print('horzEye1e10 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e10))
                print('horzEye1e12 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e12))
                print('horzEye1e15 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e15))
                print('horzEye1e17 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e17))
                print('width0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width0mUI))
                print('width100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width100mUI))
                print('horzBer0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer0mUI))
                print('horzBer100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer100mUI))
                pass
            delete_xpSerdesEyeHbtc_tp(xpSerdesEyeHbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeVbtcLogPrint
    #/********************************************************************************/
    def do_serdes_eye_vbtc_log_print(self, arg):
        '''
         xpsSerdesEyeVbtcLogPrint: Enter [ devId,serdesId,level,func,line,topIndex,bottomIndex,topPoints,bottomPoints,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 31
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,level,func,line,topIndex,bottomIndex,topPoints,bottomPoints,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            func_Ptr_3 = str(args[3])
            args[4] = int(args[4])
            xpSerdesEyeVbtc_t_Ptr = new_xpSerdesEyeVbtc_tp()
            xpSerdesEyeVbtc_t_Ptr.topIndex = int(args[5])
            xpSerdesEyeVbtc_t_Ptr.bottomIndex = int(args[6])
            xpSerdesEyeVbtc_t_Ptr.topPoints = int(args[7])
            xpSerdesEyeVbtc_t_Ptr.bottomPoints = int(args[8])
            xpSerdesEyeVbtc_t_Ptr.points = int(args[9])
            xpSerdesEyeVbtc_t_Ptr.dataColumn = int(args[10], 16)
            xpSerdesEyeVbtc_t_Ptr.totalColumns = int(args[11])
            xpSerdesEyeVbtc_t_Ptr.dcBalance = int(args[12])
            xpSerdesEyeVbtc_t_Ptr.topRj = int(args[13])
            xpSerdesEyeVbtc_t_Ptr.topSlope = int(args[14])
            xpSerdesEyeVbtc_t_Ptr.topIntercept = int(args[15])
            xpSerdesEyeVbtc_t_Ptr.topRSquared = int(args[16])
            xpSerdesEyeVbtc_t_Ptr.bottomRj = int(args[17])
            xpSerdesEyeVbtc_t_Ptr.bottomSlope = int(args[18])
            xpSerdesEyeVbtc_t_Ptr.bottomIntercept = int(args[19])
            xpSerdesEyeVbtc_t_Ptr.bottomRSquared = int(args[20])
            xpSerdesEyeVbtc_t_Ptr.vertBer0mV = int(args[21])
            xpSerdesEyeVbtc_t_Ptr.vertBer25mV = int(args[22])
            xpSerdesEyeVbtc_t_Ptr.height0mV = int(args[23])
            xpSerdesEyeVbtc_t_Ptr.height25mV = int(args[24])
            xpSerdesEyeVbtc_t_Ptr.Vmean = int(args[25])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e06 = int(args[26])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e10 = int(args[27])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e12 = int(args[28])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e15 = int(args[29])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e17 = int(args[30])
            #print('Input Arguments are, devId=%d, serdesId=%d, level=%d, func=%s, line=%d, topIndex=%d, bottomIndex=%d, topPoints=%d, bottomPoints=%d, points=%d, dataColumn=0x%x, totalColumns=%d, dcBalance=%d, topRj=%d, topSlope=%d, topIntercept=%d, topRSquared=%d, bottomRj=%d, bottomSlope=%d, bottomIntercept=%d, bottomRSquared=%d, vertBer0mV=%d, vertBer25mV=%d, height0mV=%d, height25mV=%d, Vmean=%d, vertEye1e06=%d, vertEye1e10=%d, vertEye1e12=%d, vertEye1e15=%d, vertEye1e17=%d' % (args[0],args[1],args[2],func_Ptr_3,args[4],xpSerdesEyeVbtc_t_Ptr.topIndex,xpSerdesEyeVbtc_t_Ptr.bottomIndex,xpSerdesEyeVbtc_t_Ptr.topPoints,xpSerdesEyeVbtc_t_Ptr.bottomPoints,xpSerdesEyeVbtc_t_Ptr.points,xpSerdesEyeVbtc_t_Ptr.dataColumn,xpSerdesEyeVbtc_t_Ptr.totalColumns,xpSerdesEyeVbtc_t_Ptr.dcBalance,xpSerdesEyeVbtc_t_Ptr.topRj,xpSerdesEyeVbtc_t_Ptr.topSlope,xpSerdesEyeVbtc_t_Ptr.topIntercept,xpSerdesEyeVbtc_t_Ptr.topRSquared,xpSerdesEyeVbtc_t_Ptr.bottomRj,xpSerdesEyeVbtc_t_Ptr.bottomSlope,xpSerdesEyeVbtc_t_Ptr.bottomIntercept,xpSerdesEyeVbtc_t_Ptr.bottomRSquared,xpSerdesEyeVbtc_t_Ptr.vertBer0mV,xpSerdesEyeVbtc_t_Ptr.vertBer25mV,xpSerdesEyeVbtc_t_Ptr.height0mV,xpSerdesEyeVbtc_t_Ptr.height25mV,xpSerdesEyeVbtc_t_Ptr.Vmean,xpSerdesEyeVbtc_t_Ptr.vertEye1e06,xpSerdesEyeVbtc_t_Ptr.vertEye1e10,xpSerdesEyeVbtc_t_Ptr.vertEye1e12,xpSerdesEyeVbtc_t_Ptr.vertEye1e15,xpSerdesEyeVbtc_t_Ptr.vertEye1e17))
            ret = xpsSerdesEyeVbtcLogPrint(args[0],args[1],args[2],func_Ptr_3,args[4],xpSerdesEyeVbtc_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('func = %s' % func_Ptr_3)
                print('topIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.topIndex))
                print('bottomIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIndex))
                print('topPoints = %d' % (xpSerdesEyeVbtc_t_Ptr.topPoints))
                print('bottomPoints = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomPoints))
                print('points = %d' % (xpSerdesEyeVbtc_t_Ptr.points))
                print('dataColumn = %d' % (xpSerdesEyeVbtc_t_Ptr.dataColumn))
                print('totalColumns = %d' % (xpSerdesEyeVbtc_t_Ptr.totalColumns))
                print('dcBalance = %d' % (xpSerdesEyeVbtc_t_Ptr.dcBalance))
                print('topRj = %d' % (xpSerdesEyeVbtc_t_Ptr.topRj))
                print('topSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.topSlope))
                print('topIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.topIntercept))
                print('topRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.topRSquared))
                print('bottomRj = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRj))
                print('bottomSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomSlope))
                print('bottomIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIntercept))
                print('bottomRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRSquared))
                print('vertBer0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer0mV))
                print('vertBer25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer25mV))
                print('height0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height0mV))
                print('height25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height25mV))
                print('Vmean = %d' % (xpSerdesEyeVbtc_t_Ptr.Vmean))
                print('vertEye1e06 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e06))
                print('vertEye1e10 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e10))
                print('vertEye1e12 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e12))
                print('vertEye1e15 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e15))
                print('vertEye1e17 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e17))
                pass
            delete_xpSerdesEyeVbtc_tp(xpSerdesEyeVbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeDataReadFile
    #/********************************************************************************/
    def do_serdes_eye_data_read_file(self, arg):
        '''
         xpsSerdesEyeDataReadFile: Enter [ devId,serdesId,fileName ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,fileName ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d, fileName=%s ' % (args[0],args[1],args[2]))
            ret = xpsSerdesEyeDataReadFile(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetFirmwareRev
    #/********************************************************************************/
    def do_serdes_get_firmware_rev(self, arg):
        '''
         xpsSerdesGetFirmwareRev: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fwRev_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetFirmwareRev(args[0],args[1],fwRev_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('fwRev = %d' % (uint32_tp_value(fwRev_Ptr_2)))
                pass
            delete_uint32_tp(fwRev_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetFirmwareBuildId
    #/********************************************************************************/
    def do_serdes_get_firmware_build_id(self, arg):
        '''
         xpsSerdesGetFirmwareBuildId: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fwBldId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetFirmwareBuildId(args[0],args[1],fwBldId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('fwBldId = %d' % (uint32_tp_value(fwBldId_Ptr_2)))
                pass
            delete_uint32_tp(fwBldId_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetLsbRev
    #/********************************************************************************/
    def do_serdes_get_lsb_rev(self, arg):
        '''
         xpsSerdesGetLsbRev: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetLsbRev(args[0],args[1])
            if ret < 0:
                print('Error returned = %d' % (ret))
            else:
                print('Value returned = %d' % (ret))
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxRxReady
    #/********************************************************************************/
    def do_serdes_get_tx_rx_ready(self, arg):
        '''
         xpsSerdesGetTxRxReady: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tx_Ptr_2 = new_intp()
            rx_Ptr_3 = new_intp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesGetTxRxReady(args[0],args[1],tx_Ptr_2,rx_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tx = %d' % (intp_value(tx_Ptr_2)))
                print('rx = %d' % (intp_value(rx_Ptr_3)))
                pass
            delete_intp(rx_Ptr_3)
            delete_intp(tx_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesSetTxRxEnable
    #/********************************************************************************/
    def do_serdes_set_tx_rx_enable(self, arg):
        '''
         xpsSerdesSetTxRxEnable: Enter [ devId,serdesId,tx,rx,txOutput ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,tx,rx,txOutput ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, serdesId=%d, tx=%d, rx=%d, txOutput=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsSerdesSetTxRxEnable(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxOutputEnable
    #/********************************************************************************/
    def do_serdes_get_tx_output_enable(self, arg):
        '''
         xpsSerdesGetTxOutputEnable: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_intp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesGetTxOutputEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (intp_value(enable_Ptr_2)))
                pass
            delete_intp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesSetTxOutputEnable
    #/********************************************************************************/
    def do_serdes_set_tx_output_enable(self, arg):
        '''
         xpsSerdesSetTxOutputEnable: Enter [ devId,serdesId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetTxOutputEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxRxWidth
    #/********************************************************************************/
    def do_serdes_get_tx_rx_width(self, arg):
        '''
         xpsSerdesGetTxRxWidth: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txWidth_Ptr_2 = new_int32_tp()
            rxWidth_Ptr_3 = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetTxRxWidth(args[0],args[1],txWidth_Ptr_2,rxWidth_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('txWidth = %d' % (int32_tp_value(txWidth_Ptr_2)))
                print('rxWidth = %d' % (int32_tp_value(rxWidth_Ptr_3)))
                pass
            delete_int32_tp(rxWidth_Ptr_3)
            delete_int32_tp(txWidth_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesSetTxRxWidth
    #/********************************************************************************/
    def do_serdes_set_tx_rx_width(self, arg):
        '''
         xpsSerdesSetTxRxWidth: Enter [ devId,serdesId,txWidth,rxWidth ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,txWidth,rxWidth ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, serdesId=%d, txWidth=%d, rxWidth=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesSetTxRxWidth(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetTxInvert
    #/********************************************************************************/
    def do_serdes_set_tx_invert(self, arg):
        '''
         xpsSerdesSetTxInvert: Enter [ devId,serdesId,invert ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,invert ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, invert=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetTxInvert(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetRxInvert
    #/********************************************************************************/
    def do_serdes_set_rx_invert(self, arg):
        '''
         xpsSerdesSetRxInvert: Enter [ devId,serdesId,invert ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,invert ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, invert=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetRxInvert(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesTxInjectError
    #/********************************************************************************/
    def do_serdes_tx_inject_error(self, arg):
        '''
         xpsSerdesTxInjectError: Enter [ devId,serdesId,numBits ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,numBits ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, numBits=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesTxInjectError(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesRxInjectError
    #/********************************************************************************/
    def do_serdes_rx_inject_error(self, arg):
        '''
         xpsSerdesRxInjectError: Enter [ devId,serdesId,numBits ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,numBits ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, numBits=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesRxInjectError(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetRxInputLoopback
    #/********************************************************************************/
    def do_serdes_set_rx_input_loopback(self, arg):
        '''
         xpsSerdesSetRxInputLoopback: Enter [ devId,serdesId,selectInternal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,selectInternal ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, selectInternal=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetRxInputLoopback(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxUserData
    #/********************************************************************************/
    def do_serdes_get_tx_user_data(self, arg):
        '''
         xpsSerdesGetTxUserData: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            data = [0] * 4
            print('========================')
            print(' serdes-Id |    Data   |')
            print('========================')
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetTxUserData(args[0],serdesId,data)
                if ret != 0:
                    print('Error : Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('    %03d    |  0x%05X  |' % (serdesId, data[0]))
            del data
            print('========================')

    #/********************************************************************************/
    # command for xpsSerdesSetTxUserData
    #/********************************************************************************/
    def do_serdes_set_tx_user_data(self, arg):
        '''
         xpsSerdesSetTxUserData: Enter [ devId,serdesId/startSerdesId-endSerdesId,data ]
         Example: serdes_set_tx_user_data 0 10 0x04 / serdes_set_tx_user_data 0 10-139 0x04
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,data ]')
            print('Example: serdes_set_tx_user_data 0 10 0x04 / serdes_set_tx_user_data 0 10-139 0x04')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            try:
                args[2] = int(args[2],16)
            except:
                print('Error: data should be in hex')
                return
            data = [0] * 4
            data[3] = args[2]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d, data=%s' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSetTxUserData(args[0],serdesId,data)
                if ret != 0:
                    print('Error: Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('Tx userdata set successfully for serdesId = %d' % (serdesId))
                    pass

    #/********************************************************************************/
    # command for xpsSerdesSetTxDataSel
    #/********************************************************************************/
    def do_serdes_set_tx_data_sel(self, arg):
        '''
         xpsSerdesSetTxDataSel: Enter [ devId,serdesId,input ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,input ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, input=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetTxDataSel(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetRxCmpData
    #/********************************************************************************/
    def do_serdes_set_rx_cmp_data(self, arg):
        '''
         xpsSerdesSetRxCmpData: Enter [ devId,serdesId,cmpData ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,cmpData ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, cmpData=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetRxCmpData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxEq
    #/********************************************************************************/
    def do_serdes_get_tx_eq(self, arg):
        '''
         xpsSerdesGetTxEq: Enter [ devId,serdesId,pre,atten,post,slew ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,pre,atten,post,slew ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesTxEq_t_Ptr = new_xpSerdesTxEq_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesGetTxEq(args[0],args[1],xpSerdesTxEq_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('pre = %d' % (xpSerdesTxEq_t_Ptr.pre))
                print('atten = %d' % (xpSerdesTxEq_t_Ptr.atten))
                print('post = %d' % (xpSerdesTxEq_t_Ptr.post))
                print('slew = %d' % (xpSerdesTxEq_t_Ptr.slew))
                pass
            delete_xpSerdesTxEq_tp(xpSerdesTxEq_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesSetTxEq
    #/********************************************************************************/
    def do_serdes_set_tx_eq(self, arg):
        '''
         xpsSerdesSetTxEq: Enter [ devId,serdesId,pre,atten,post,slew ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,pre,atten,post,slew ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesTxEq_t_Ptr = new_xpSerdesTxEq_tp()
            xpSerdesTxEq_t_Ptr.pre = int(args[2])
            xpSerdesTxEq_t_Ptr.atten = int(args[3])
            xpSerdesTxEq_t_Ptr.post = int(args[4])
            xpSerdesTxEq_t_Ptr.slew = int(args[5])
            #print('Input Arguments are, devId=%d, serdesId=%d, pre=%d, atten=%d, post=%d, slew=%d' % (args[0],args[1],xpSerdesTxEq_t_Ptr.pre,xpSerdesTxEq_t_Ptr.atten,xpSerdesTxEq_t_Ptr.post,xpSerdesTxEq_t_Ptr.slew))
            ret = xpsSerdesSetTxEq(args[0],args[1],xpSerdesTxEq_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('pre = %d' % (xpSerdesTxEq_t_Ptr.pre))
                print('atten = %d' % (xpSerdesTxEq_t_Ptr.atten))
                print('post = %d' % (xpSerdesTxEq_t_Ptr.post))
                print('slew = %d' % (xpSerdesTxEq_t_Ptr.slew))
                pass
            delete_xpSerdesTxEq_tp(xpSerdesTxEq_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesSetRxTerm
    #/********************************************************************************/
    def do_serdes_set_rx_term(self, arg):
        '''
         xpsSerdesSetRxTerm: Enter [ devId,serdesId,term ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,term ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, term=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetRxTerm(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetRxCmpMode
    #/********************************************************************************/
    def do_serdes_set_rx_cmp_mode(self, arg):
        '''
         xpsSerdesSetRxCmpMode: Enter [ devId,serdesId,mode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,mode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, mode=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetRxCmpMode(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetTxPllClkSrc
    #/********************************************************************************/
    def do_serdes_set_tx_pll_clk_src(self, arg):
        '''
         xpsSerdesSetTxPllClkSrc: Enter [ devId,serdesId,src ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,src ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, src=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetTxPllClkSrc(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesMemRd
    #/********************************************************************************/
    def do_serdes_mem_rd(self, arg):
        '''
         xpsSerdesMemRd: Enter [ devId,serdesId,type,memAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,type,memAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3], 16)
            #print('Input Arguments are, devId=%d, serdesId=%d, type=%d, memAddr=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesMemRd(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesMemWr
    #/********************************************************************************/
    def do_serdes_mem_wr(self, arg):
        '''
         xpsSerdesMemWr: Enter [ devId,serdesId,type,memAddr,data ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,type,memAddr,data ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4], 16)
            #print('Input Arguments are, devId=%d, serdesId=%d, type=%d, memAddr=0x%x, data=0x%x' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsSerdesMemWr(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesMemRmw
    #/********************************************************************************/
    def do_serdes_mem_rmw(self, arg):
        '''
         xpsSerdesMemRmw: Enter [ devId,serdesId,type,memAaddr,data,mask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,type,memAaddr,data,mask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3], 16)
            args[4] = int(args[4], 16)
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, serdesId=%d, type=%d, memAddr=0x%x, data=0x%x, mask=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsSerdesMemRmw(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret < 0:
                print('Error returned = %d' % (ret))
            else:
                print('Initial Value = %d' % (ret))
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetSignalOk
    #/********************************************************************************/
    def do_serdes_get_signal_ok(self, arg):
        '''
         xpsSerdesGetSignalOk: Enter [ devId,serdesId,reset ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,reset ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            signalOk_Ptr_3 = new_intp()
            #print('Input Arguments are, devId=%d, serdesId=%d, reset=%d' % (args[0],args[1],args[2]))

            ret = xpsSerdesGetSignalOk(args[0],args[1],args[2],signalOk_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('signalOk = %d' % (intp_value(signalOk_Ptr_3)))
                pass
            delete_intp(signalOk_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesInitializeSignalOk
    #/********************************************************************************/
    def do_serdes_initialize_signal_ok(self, arg):
        '''
         xpsSerdesInitializeSignalOk: Enter [ devId,serdesId,threshold ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,threshold ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, threshold=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesInitializeSignalOk(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetErrors
    #/********************************************************************************/
    def do_serdes_get_errors(self, arg):
        '''
         xpsSerdesGetErrors: Enter [ devId,serdesId,type,resetCountAfterGet ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,type,resetCountAfterGet ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            errCnt_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, type=%d, resetCountAfterGet=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesGetErrors(args[0],args[1],args[2],args[3],errCnt_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('errCnt = %d' % (uint32_tp_value(errCnt_Ptr_4)))
                pass
            delete_uint32_tp(errCnt_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesGetTxInvert
    #/********************************************************************************/
    def do_serdes_get_tx_invert(self, arg):
        '''
         xpsSerdesGetTxInvert: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            getTxInvert_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetTxInvert(args[0],args[1],getTxInvert_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('getTxInvert = %d' % (uint8_tp_value(getTxInvert_Ptr_2)))
                pass
            delete_uint8_tp(getTxInvert_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetRxInvert
    #/********************************************************************************/
    def do_serdes_get_rx_invert(self, arg):
        '''
         xpsSerdesGetRxInvert: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            getRxInvert_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxInvert(args[0],args[1],getRxInvert_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('getRxInvert = %d' % (uint8_tp_value(getRxInvert_Ptr_2)))
                pass
            delete_uint8_tp(getRxInvert_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetRxInputLoopback
    #/********************************************************************************/
    def do_serdes_get_rx_input_loopback(self, arg):
        '''
         xpsSerdesGetRxInputLoopback: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            getRxInputLoopback_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxInputLoopback(args[0],args[1],getRxInputLoopback_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('getRxInputLoopback = %d' % (uint8_tp_value(getRxInputLoopback_Ptr_2)))
                pass
            delete_uint8_tp(getRxInputLoopback_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetTxDataSel
    #/********************************************************************************/
    def do_serdes_get_tx_data_sel(self, arg):
        '''
         xpsSerdesGetTxDataSel: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txDataSel_Ptr_2 = new_xpSerdesTxDataSel_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetTxDataSel(args[0],args[1],txDataSel_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('txDataSel = %d' % (xpSerdesTxDataSel_tp_value(txDataSel_Ptr_2)))
                pass
            delete_xpSerdesTxDataSel_tp(txDataSel_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetRxCmpData
    #/********************************************************************************/
    def do_serdes_get_rx_cmp_data(self, arg):
        '''
         xpsSerdesGetRxCmpData: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxCmpData_Ptr_2 = new_xpSerdesRxCmpData_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxCmpData(args[0],args[1],rxCmpData_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rxCmpData = %d' % (xpSerdesRxCmpData_tp_value(rxCmpData_Ptr_2)))
                pass
            delete_xpSerdesRxCmpData_tp(rxCmpData_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetRxTerm
    #/********************************************************************************/
    def do_serdes_get_rx_term(self, arg):
        '''
         xpsSerdesGetRxTerm: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            getRxTerm_Ptr_2 = new_xpSerdesRxTerm_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxTerm(args[0],args[1],getRxTerm_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('getRxTerm = %d' % (xpSerdesRxTerm_tp_value(getRxTerm_Ptr_2)))
                pass
            delete_xpSerdesRxTerm_tp(getRxTerm_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetRxCmpMode
    #/********************************************************************************/
    def do_serdes_get_rx_cmp_mode(self, arg):
        '''
         xpsSerdesGetRxCmpMode: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rxCmpModeT_Ptr_2 = new_xpSerdesRxCmpMode_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxCmpMode(args[0],args[1],rxCmpModeT_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rxCmpModeT = %d' % (xpSerdesRxCmpMode_tp_value(rxCmpModeT_Ptr_2)))
                pass
            delete_xpSerdesRxCmpMode_tp(rxCmpModeT_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetTxPllClkSrc
    #/********************************************************************************/
    def do_serdes_get_tx_pll_clk_src(self, arg):
        '''
         xpsSerdesGetTxPllClkSrc: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            txPllClkT_Ptr_2 = new_xpSerdesTxPllClk_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetTxPllClkSrc(args[0],args[1],txPllClkT_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('txPllClkT = %d' % (xpSerdesTxPllClk_tp_value(txPllClkT_Ptr_2)))
                pass
            delete_xpSerdesTxPllClk_tp(txPllClkT_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesEyeDataWriteFile
    #/********************************************************************************/
    def do_serdes_eye_data_write_file(self, arg):
        '''
         xpsSerdesEyeDataWriteFile: Enter [ devId,serdesId,fileName ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,fileName ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d, fileName=%s' % (args[0],args[1],args[2]))
            ret = xpsSerdesEyeDataWriteFile(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesEyeDataWrite
    #/********************************************************************************/
    def do_serdes_eye_data_write(self, arg):
        '''
         xpsSerdesEyeDataWrite: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            file_Ptr_2 = new_FILEp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesEyeDataWrite(args[0],args[1],file_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('file = %d' % (FILEp_value(file_Ptr_2)))
                pass
            delete_FILEp(file_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesEyeVbtcWrite
    #/********************************************************************************/
    def do_serdes_eye_vbtc_write(self, arg):
        '''
         xpsSerdesEyeVbtcWrite: Enter [ devId,serdesId,file]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,file]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fileName = args[2]
            ret = xpsSerdesEyeVbtcWrite(args[0],args[1],fileName)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1

    #/********************************************************************************/
    # command for xpsSerdesEyeHbtcWrite
    #/********************************************************************************/
    def do_serdes_eye_hbtc_write(self, arg):
        '''
         xpsSerdesEyeHbtcWrite: Enter [ devId,serdesId,file]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,file]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fileName = args[2]
            ret = xpsSerdesEyeHbtcWrite(args[0],args[1],fileName)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1

    #/********************************************************************************/
    # command for xpsSerdesEyePlotWrite
    #/********************************************************************************/
    def do_serdes_eye_plot_write(self, arg):
        '''
         xpsSerdesEyePlotWrite: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            file_Ptr_2 = new_FILEp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesEyePlotWrite(args[0],args[1],file_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('file = %d' % (FILEp_value(file_Ptr_2)))
                pass
            delete_FILEp(file_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesEyeVbtcFormat
    #/********************************************************************************/
    def do_serdes_eye_vbtc_format(self, arg):
        '''
         xpsSerdesEyeVbtcFormat: Enter [ devId,serdesId,eyeData,topIndex,bottomIndex,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,eyeData,topIndex,bottomIndex,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeData_ptr = str(args[2])
            xpSerdesEyeVbtc_t_Ptr = new_xpSerdesEyeVbtc_tp()
            xpSerdesEyeVbtc_t_Ptr.topIndex = int(args[3])
            xpSerdesEyeVbtc_t_Ptr.bottomIndex = int(args[4])
            xpSerdesEyeVbtc_t_Ptr.points = int(args[5])
            xpSerdesEyeVbtc_t_Ptr.dataColumn = int(args[6], 16)
            xpSerdesEyeVbtc_t_Ptr.totalColumns = int(args[7])
            xpSerdesEyeVbtc_t_Ptr.dcBalance = int(args[8])
            xpSerdesEyeVbtc_t_Ptr.topRj = int(args[9])
            xpSerdesEyeVbtc_t_Ptr.topSlope = int(args[10])
            xpSerdesEyeVbtc_t_Ptr.topIntercept = int(args[11])
            xpSerdesEyeVbtc_t_Ptr.topRSquared = int(args[12])
            xpSerdesEyeVbtc_t_Ptr.bottomRj = int(args[13])
            xpSerdesEyeVbtc_t_Ptr.bottomSlope = int(args[14])
            xpSerdesEyeVbtc_t_Ptr.bottomIntercept = int(args[15])
            xpSerdesEyeVbtc_t_Ptr.bottomRSquared = int(args[16])
            xpSerdesEyeVbtc_t_Ptr.vertBer0mV = int(args[17])
            xpSerdesEyeVbtc_t_Ptr.vertBer25mV = int(args[18])
            xpSerdesEyeVbtc_t_Ptr.height0mV = int(args[19])
            xpSerdesEyeVbtc_t_Ptr.height25mV = int(args[20])
            xpSerdesEyeVbtc_t_Ptr.Vmean = int(args[21])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e06 = int(args[22])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e10 = int(args[23])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e12 = int(args[24])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e15 = int(args[25])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e17 = int(args[26])
            #print('Input Arguments are, devId=%d, serdesId=%d, eyeData=%s, topIndex=%d, bottomIndex=%d, points=%d, dataColumn=0x%x, totalColumns=%d, dcBalance=%d, topRj=%d, topSlope=%d, topIntercept=%d, topRSquared=%d, bottomRj=%d, bottomSlope=%d, bottomIntercept=%d, bottomRSquared=%d, vertBer0mV=%d, vertBer25mV=%d, height0mV=%d, height25mV=%d, Vmean=%d, vertEye1e06=%d, vertEye1e10=%d, vertEye1e12=%d, vertEye1e15=%d, vertEye1e17=%d' % (args[0],args[1],eyeData_ptr,xpSerdesEyeVbtc_t_Ptr.topIndex,xpSerdesEyeVbtc_t_Ptr.bottomIndex,xpSerdesEyeVbtc_t_Ptr.points,xpSerdesEyeVbtc_t_Ptr.dataColumn,xpSerdesEyeVbtc_t_Ptr.totalColumns,xpSerdesEyeVbtc_t_Ptr.dcBalance,xpSerdesEyeVbtc_t_Ptr.topRj,xpSerdesEyeVbtc_t_Ptr.topSlope,xpSerdesEyeVbtc_t_Ptr.topIntercept,xpSerdesEyeVbtc_t_Ptr.topRSquared,xpSerdesEyeVbtc_t_Ptr.bottomRj,xpSerdesEyeVbtc_t_Ptr.bottomSlope,xpSerdesEyeVbtc_t_Ptr.bottomIntercept,xpSerdesEyeVbtc_t_Ptr.bottomRSquared,xpSerdesEyeVbtc_t_Ptr.vertBer0mV,xpSerdesEyeVbtc_t_Ptr.vertBer25mV,xpSerdesEyeVbtc_t_Ptr.height0mV,xpSerdesEyeVbtc_t_Ptr.height25mV,xpSerdesEyeVbtc_t_Ptr.Vmean,xpSerdesEyeVbtc_t_Ptr.vertEye1e06,xpSerdesEyeVbtc_t_Ptr.vertEye1e10,xpSerdesEyeVbtc_t_Ptr.vertEye1e12,xpSerdesEyeVbtc_t_Ptr.vertEye1e15,xpSerdesEyeVbtc_t_Ptr.vertEye1e17))
            ret = xpsSerdesEyeVbtcFormat(args[0],args[1],xpSerdesEyeVbtc_t_Ptr,eyeData_ptr)
            if ret == 0:
                print('topIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.topIndex))
                print('bottomIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIndex))
                print('points = %d' % (xpSerdesEyeVbtc_t_Ptr.points))
                print('dataColumn = %d' % (xpSerdesEyeVbtc_t_Ptr.dataColumn))
                print('totalColumns = %d' % (xpSerdesEyeVbtc_t_Ptr.totalColumns))
                print('dcBalance = %d' % (xpSerdesEyeVbtc_t_Ptr.dcBalance))
                print('topRj = %d' % (xpSerdesEyeVbtc_t_Ptr.topRj))
                print('topSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.topSlope))
                print('topIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.topIntercept))
                print('topRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.topRSquared))
                print('bottomRj = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRj))
                print('bottomSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomSlope))
                print('bottomIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIntercept))
                print('bottomRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRSquared))
                print('vertBer0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer0mV))
                print('vertBer25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer25mV))
                print('height0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height0mV))
                print('height25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height25mV))
                print('Vmean = %d' % (xpSerdesEyeVbtc_t_Ptr.Vmean))
                print('vertEye1e06 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e06))
                print('vertEye1e10 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e10))
                print('vertEye1e12 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e12))
                print('vertEye1e15 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e15))
                print('vertEye1e17 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e17))
                pass
            delete_xpSerdesEyeVbtc_tp(xpSerdesEyeVbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeHbtcFormat
    #/********************************************************************************/
    def do_serdes_eye_hbtc_format(self, arg):
        '''
         xpsSerdesEyeHbtcFormat: Enter [ devId,serdesId,eyeData,leftIndex,rightIndex,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,eyeData,leftIndex,rightIndex,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeData_ptr = str(args[2])
            xpSerdesEyeHbtc_t_Ptr = new_xpSerdesEyeHbtc_tp()
            xpSerdesEyeHbtc_t_Ptr.leftIndex = int(args[3])
            xpSerdesEyeHbtc_t_Ptr.rightIndex = int(args[4])
            xpSerdesEyeHbtc_t_Ptr.points = int(args[5])
            xpSerdesEyeHbtc_t_Ptr.dataRow = int(args[6], 16)
            xpSerdesEyeHbtc_t_Ptr.totalRows = int(args[7])
            xpSerdesEyeHbtc_t_Ptr.transDensity = int(args[8])
            xpSerdesEyeHbtc_t_Ptr.dj = int(args[9])
            xpSerdesEyeHbtc_t_Ptr.leftRj = int(args[10])
            xpSerdesEyeHbtc_t_Ptr.leftSlope = int(args[11])
            xpSerdesEyeHbtc_t_Ptr.leftIntercept = int(args[12])
            xpSerdesEyeHbtc_t_Ptr.leftRSquared = int(args[13])
            xpSerdesEyeHbtc_t_Ptr.rightRj = int(args[14])
            xpSerdesEyeHbtc_t_Ptr.rightSlope = int(args[15])
            xpSerdesEyeHbtc_t_Ptr.rightIntercept = int(args[16])
            xpSerdesEyeHbtc_t_Ptr.rightRSquared = int(args[17])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e06 = int(args[18])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e10 = int(args[19])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e12 = int(args[20])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e15 = int(args[21])
            xpSerdesEyeHbtc_t_Ptr.horzEye1e17 = int(args[22])
            xpSerdesEyeHbtc_t_Ptr.width0mUI = int(args[23])
            xpSerdesEyeHbtc_t_Ptr.width100mUI = int(args[24])
            xpSerdesEyeHbtc_t_Ptr.horzBer0mUI = int(args[25])
            xpSerdesEyeHbtc_t_Ptr.horzBer100mUI = int(args[26])
            #print('Input Arguments are, devId=%d, serdesId=%d, eyeData=%s, leftIndex=%d, rightIndex=%d, points=%d, dataRow=0x%x, totalRows=%d, transDensity=%d, dj=%d, leftRj=%d, leftSlope=%d, leftIntercept=%d, leftRSquared=%d, rightRj=%d, rightSlope=%d, rightIntercept=%d, rightRSquared=%d, horzEye1e06=%d, horzEye1e10=%d, horzEye1e12=%d, horzEye1e15=%d, horzEye1e17=%d, width0mUI=%d, width100mUI=%d, horzBer0mUI=%d, horzBer100mUI=%d' % (args[0],args[1],eyeData_ptr,xpSerdesEyeHbtc_t_Ptr.leftIndex,xpSerdesEyeHbtc_t_Ptr.rightIndex,xpSerdesEyeHbtc_t_Ptr.points,xpSerdesEyeHbtc_t_Ptr.dataRow,xpSerdesEyeHbtc_t_Ptr.totalRows,xpSerdesEyeHbtc_t_Ptr.transDensity,xpSerdesEyeHbtc_t_Ptr.dj,xpSerdesEyeHbtc_t_Ptr.leftRj,xpSerdesEyeHbtc_t_Ptr.leftSlope,xpSerdesEyeHbtc_t_Ptr.leftIntercept,xpSerdesEyeHbtc_t_Ptr.leftRSquared,xpSerdesEyeHbtc_t_Ptr.rightRj,xpSerdesEyeHbtc_t_Ptr.rightSlope,xpSerdesEyeHbtc_t_Ptr.rightIntercept,xpSerdesEyeHbtc_t_Ptr.rightRSquared,xpSerdesEyeHbtc_t_Ptr.horzEye1e06,xpSerdesEyeHbtc_t_Ptr.horzEye1e10,xpSerdesEyeHbtc_t_Ptr.horzEye1e12,xpSerdesEyeHbtc_t_Ptr.horzEye1e15,xpSerdesEyeHbtc_t_Ptr.horzEye1e17,xpSerdesEyeHbtc_t_Ptr.width0mUI,xpSerdesEyeHbtc_t_Ptr.width100mUI,xpSerdesEyeHbtc_t_Ptr.horzBer0mUI,xpSerdesEyeHbtc_t_Ptr.horzBer100mUI))
            ret = xpsSerdesEyeHbtcFormat(args[0],args[1],xpSerdesEyeHbtc_t_Ptr,eyeData_ptr)
            if ret == 0:
                print('leftIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIndex))
                print('rightIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIndex))
                print('points = %d' % (xpSerdesEyeHbtc_t_Ptr.points))
                print('dataRow = %d' % (xpSerdesEyeHbtc_t_Ptr.dataRow))
                print('totalRows = %d' % (xpSerdesEyeHbtc_t_Ptr.totalRows))
                print('transDensity = %d' % (xpSerdesEyeHbtc_t_Ptr.transDensity))
                print('dj = %d' % (xpSerdesEyeHbtc_t_Ptr.dj))
                print('leftRj = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRj))
                print('leftSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.leftSlope))
                print('leftIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIntercept))
                print('leftRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRSquared))
                print('rightRj = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRj))
                print('rightSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.rightSlope))
                print('rightIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIntercept))
                print('rightRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRSquared))
                print('horzEye1e06 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e06))
                print('horzEye1e10 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e10))
                print('horzEye1e12 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e12))
                print('horzEye1e15 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e15))
                print('horzEye1e17 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e17))
                print('width0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width0mUI))
                print('width100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width100mUI))
                print('horzBer0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer0mUI))
                print('horzBer100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer100mUI))
                pass
            delete_xpSerdesEyeHbtc_tp(xpSerdesEyeHbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyePlotFormat
    #/********************************************************************************/
    def do_serdes_eye_plot_format(self, arg):
        '''
         xpsSerdesEyePlotFormat: Enter [ devId,serdesId,eyeData ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,eyeData ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeData_ptr = str(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, eyeData=%s' % (args[0],args[1],eyeData_ptr))
            ret = xpsSerdesEyePlotFormat(args[0],args[1],eyeData_ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeVbtcExtrapolate
    #/********************************************************************************/
    def do_serdes_eye_vbtc_extrapolate(self, arg):
        '''
         xpsSerdesEyeVbtcExtrapolate: Enter [ devId,serdesId,dataCol,topIndex,bottomIndex,topPoints,bottomPoints,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 29
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,dataCol,topIndex,bottomIndex,topPoints,bottomPoints,points,dataColumn,totalColumns,dcBalance,topRj,topSlope,topIntercept,topRSquared,bottomRj,bottomSlope,bottomIntercept,bottomRSquared,vertBer0mV,vertBer25mV,height0mV,height25mV,Vmean,vertEye1e06,vertEye1e10,vertEye1e12,vertEye1e15,vertEye1e17 ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            xpSerdesEyeVbtc_t_Ptr = new_xpSerdesEyeVbtc_tp()

            xpSerdesEyeVbtc_t_Ptr.topIndex = int(args[3])
            xpSerdesEyeVbtc_t_Ptr.bottomIndex = int(args[4])
            xpSerdesEyeVbtc_t_Ptr.topPoints = int(args[5])
            xpSerdesEyeVbtc_t_Ptr.bottomPoints = int(args[6])
            xpSerdesEyeVbtc_t_Ptr.points = int(args[7])
            xpSerdesEyeVbtc_t_Ptr.dataColumn = int(args[8], 16)
            xpSerdesEyeVbtc_t_Ptr.totalColumns = int(args[9])
            xpSerdesEyeVbtc_t_Ptr.dcBalance = int(args[10])
            xpSerdesEyeVbtc_t_Ptr.topRj = int(args[11])
            xpSerdesEyeVbtc_t_Ptr.topSlope = int(args[12])
            xpSerdesEyeVbtc_t_Ptr.topIntercept = int(args[13])
            xpSerdesEyeVbtc_t_Ptr.topRSquared = int(args[14])
            xpSerdesEyeVbtc_t_Ptr.bottomRj = int(args[15])
            xpSerdesEyeVbtc_t_Ptr.bottomSlope = int(args[16])
            xpSerdesEyeVbtc_t_Ptr.bottomIntercept = int(args[17])
            xpSerdesEyeVbtc_t_Ptr.bottomRSquared = int(args[18])
            xpSerdesEyeVbtc_t_Ptr.vertBer0mV = int(args[19])
            xpSerdesEyeVbtc_t_Ptr.vertBer25mV = int(args[20])
            xpSerdesEyeVbtc_t_Ptr.height0mV = int(args[21])
            xpSerdesEyeVbtc_t_Ptr.height25mV = int(args[22])
            xpSerdesEyeVbtc_t_Ptr.Vmean = int(args[23])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e06 = int(args[24])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e10 = int(args[25])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e12 = int(args[26])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e15 = int(args[27])
            xpSerdesEyeVbtc_t_Ptr.vertEye1e17 = int(args[28])
            #print('Input Arguments are, devId=%d, serdesId=%d, dataCol=0x%x, topIndex=%d, bottomIndex=%d, topPoints=%d, bottomPoints=%d, points=%d, dataColumn=0x%x, totalColumns=%d, dcBalance=%d, topRj=%d, topSlope=%d, topIntercept=%d, topRSquared=%d, bottomRj=%d, bottomSlope=%d, bottomIntercept=%d, bottomRSquared=%d, vertBer0mV=%d, vertBer25mV=%d, height0mV=%d, height25mV=%d, Vmean=%d, vertEye1e06=%d, vertEye1e10=%d, vertEye1e12=%d, vertEye1e15=%d, vertEye1e17=%d' % (args[0],args[1],args[2],xpSerdesEyeVbtc_t_Ptr.topIndex,xpSerdesEyeVbtc_t_Ptr.bottomIndex,xpSerdesEyeVbtc_t_Ptr.topPoints,xpSerdesEyeVbtc_t_Ptr.bottomPoints,xpSerdesEyeVbtc_t_Ptr.points,xpSerdesEyeVbtc_t_Ptr.dataColumn,xpSerdesEyeVbtc_t_Ptr.totalColumns,xpSerdesEyeVbtc_t_Ptr.dcBalance,xpSerdesEyeVbtc_t_Ptr.topRj,xpSerdesEyeVbtc_t_Ptr.topSlope,xpSerdesEyeVbtc_t_Ptr.topIntercept,xpSerdesEyeVbtc_t_Ptr.topRSquared,xpSerdesEyeVbtc_t_Ptr.bottomRj,xpSerdesEyeVbtc_t_Ptr.bottomSlope,xpSerdesEyeVbtc_t_Ptr.bottomIntercept,xpSerdesEyeVbtc_t_Ptr.bottomRSquared,xpSerdesEyeVbtc_t_Ptr.vertBer0mV,xpSerdesEyeVbtc_t_Ptr.vertBer25mV,xpSerdesEyeVbtc_t_Ptr.height0mV,xpSerdesEyeVbtc_t_Ptr.height25mV,xpSerdesEyeVbtc_t_Ptr.Vmean,xpSerdesEyeVbtc_t_Ptr.vertEye1e06,xpSerdesEyeVbtc_t_Ptr.vertEye1e10,xpSerdesEyeVbtc_t_Ptr.vertEye1e12,xpSerdesEyeVbtc_t_Ptr.vertEye1e15,xpSerdesEyeVbtc_t_Ptr.vertEye1e17))

            ret = xpsSerdesEyeVbtcExtrapolate(args[0],args[1],args[2],xpSerdesEyeVbtc_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('topIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.topIndex))
                print('bottomIndex = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIndex))
                print('topPoints = %d' % (xpSerdesEyeVbtc_t_Ptr.topPoints))
                print('bottomPoints = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomPoints))
                print('points = %d' % (xpSerdesEyeVbtc_t_Ptr.points))
                print('dataColumn = %d' % (xpSerdesEyeVbtc_t_Ptr.dataColumn))
                print('totalColumns = %d' % (xpSerdesEyeVbtc_t_Ptr.totalColumns))
                print('dcBalance = %d' % (xpSerdesEyeVbtc_t_Ptr.dcBalance))
                print('topRj = %d' % (xpSerdesEyeVbtc_t_Ptr.topRj))
                print('topSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.topSlope))
                print('topIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.topIntercept))
                print('topRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.topRSquared))
                print('bottomRj = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRj))
                print('bottomSlope = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomSlope))
                print('bottomIntercept = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomIntercept))
                print('bottomRSquared = %d' % (xpSerdesEyeVbtc_t_Ptr.bottomRSquared))
                print('vertBer0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer0mV))
                print('vertBer25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.vertBer25mV))
                print('height0mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height0mV))
                print('height25mV = %d' % (xpSerdesEyeVbtc_t_Ptr.height25mV))
                print('Vmean = %d' % (xpSerdesEyeVbtc_t_Ptr.Vmean))
                print('vertEye1e06 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e06))
                print('vertEye1e10 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e10))
                print('vertEye1e12 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e12))
                print('vertEye1e15 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e15))
                print('vertEye1e17 = %d' % (xpSerdesEyeVbtc_t_Ptr.vertEye1e17))
                pass
            delete_xpSerdesEyeVbtc_tp(xpSerdesEyeVbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeHbtcExtrapolate
    #/********************************************************************************/
    def do_serdes_eye_hbtc_extrapolate(self, arg):
        '''
         xpsSerdesEyeHbtcExtrapolate: Enter [ devId,serdesId,dataRow,leftIndex,rightIndex,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 26
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,dataRow,leftIndex,rightIndex,points,dataRow,totalRows,transDensity,dj,leftRj,leftSlope,leftIntercept,leftRSquared,rightRj,rightSlope,rightIntercept,rightRSquared,horzEye1e06,horzEye1e10,horzEye1e12,horzEye1e15,horzEye1e17,width0mUI,width100mUI,horzBer0mUI,horzBer100mUI ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            xpSerdesEyeHbtc_t_Ptr = new_xpSerdesEyeHbtc_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, dataRow=0x%x' % (args[0],args[1],args[2]))

            ret = xpsSerdesEyeHbtcExtrapolate(args[0],args[1],args[2],xpSerdesEyeHbtc_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('leftIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIndex))
                print('rightIndex = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIndex))
                print('points = %d' % (xpSerdesEyeHbtc_t_Ptr.points))
                print('dataRow = %d' % (xpSerdesEyeHbtc_t_Ptr.dataRow))
                print('totalRows = %d' % (xpSerdesEyeHbtc_t_Ptr.totalRows))
                print('transDensity = %d' % (xpSerdesEyeHbtc_t_Ptr.transDensity))
                print('dj = %d' % (xpSerdesEyeHbtc_t_Ptr.dj))
                print('leftRj = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRj))
                print('leftSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.leftSlope))
                print('leftIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.leftIntercept))
                print('leftRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.leftRSquared))
                print('rightRj = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRj))
                print('rightSlope = %d' % (xpSerdesEyeHbtc_t_Ptr.rightSlope))
                print('rightIntercept = %d' % (xpSerdesEyeHbtc_t_Ptr.rightIntercept))
                print('rightRSquared = %d' % (xpSerdesEyeHbtc_t_Ptr.rightRSquared))
                print('horzEye1e06 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e06))
                print('horzEye1e10 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e10))
                print('horzEye1e12 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e12))
                print('horzEye1e15 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e15))
                print('horzEye1e17 = %d' % (xpSerdesEyeHbtc_t_Ptr.horzEye1e17))
                print('width0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width0mUI))
                print('width100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.width100mUI))
                print('horzBer0mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer0mUI))
                print('horzBer100mUI = %d' % (xpSerdesEyeHbtc_t_Ptr.horzBer100mUI))
                pass
            delete_xpSerdesEyeHbtc_tp(xpSerdesEyeHbtc_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrStatus
    #/********************************************************************************/
    def do_serdes_ctrlr_status(self, arg):
        '''
         xpsSerdesCtrlrStatus: Enter [ devId,serdesId,enabled,pc,revision,build,clk,state ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,enabled,pc,revision,build,clk,state ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesStatus_t_Ptr = new_xpSerdesStatus_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesCtrlrStatus(args[0],args[1],xpSerdesStatus_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enabled = %d' % (xpSerdesStatus_t_Ptr.enabled))
                print('pc = %d' % (xpSerdesStatus_t_Ptr.pc))
                print('revision = %d' % (xpSerdesStatus_t_Ptr.revision))
                print('build = %d' % (xpSerdesStatus_t_Ptr.build))
                print('clk = %d' % (xpSerdesStatus_t_Ptr.clk))
                print('state = %d' % (xpSerdesStatus_t_Ptr.state))
                pass
            delete_xpSerdesStatus_tp(xpSerdesStatus_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrRunning
    #/********************************************************************************/
    def do_serdes_ctrlr_running(self, arg):
        '''
         xpsSerdesCtrlrRunning: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            runStatus_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesCtrlrRunning(args[0],args[1],runStatus_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('runStatus = %d' % (uint32_tp_value(runStatus_Ptr_2)))
                pass
            delete_uint32_tp(runStatus_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrReset
    #/********************************************************************************/
    def do_serdes_ctrlr_reset(self, arg):
        '''
         xpsSerdesCtrlrReset: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesCtrlrReset(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesCtrlrUploadSwapImage
    #/********************************************************************************/
    def do_serdes_ctrlr_upload_swap_image(self, arg):
        '''
         xpsSerdesCtrlrUploadSwapImage: Enter [ devId,serdesId,words ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,words ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            rom_Ptr_3 = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, words=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesCtrlrUploadSwapImage(args[0],args[1],args[2],rom_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rom = %d' % (int32_tp_value(rom_Ptr_3)))
                pass
            delete_int32_tp(rom_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesFirmwareUpload
    #/********************************************************************************/
    def do_serdes_firmware_upload(self, arg):
        '''
         xpsSerdesFirmwareUpload: Enter [ devId,serdesId,ramBist,words ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,ramBist,words ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            rom_Ptr_4 = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, ramBist=%d, words=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesFirmwareUpload(args[0],args[1],args[2],args[3],rom_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('rom = %d' % (int32_tp_value(rom_Ptr_4)))
                pass
            delete_int32_tp(rom_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrInt
    #/********************************************************************************/
    def do_serdes_firmware_int(self, arg):
        '''
         xpsSerdesCtrlrInt: Enter [ devId,serdesId,intNum,param ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,intNum,param ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            intStatus_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, intNum=%d, param=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesCtrlrInt(args[0],args[1],args[2],args[3],intStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intStatus = %d' % (uint32_tp_value(intStatus_Ptr_4)))
                pass
            delete_uint32_tp(intStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrIntArray
    #/********************************************************************************/
    def do_serdes_ctrlr_int_array(self, arg):
        '''
         xpsSerdesCtrlrIntArray: Enter [ devId,serdesId,numElements,interrupt,param,ret,flags ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,numElements,interrupt,param,ret,flags ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpSerdesInt_t_Ptr = new_xpSerdesInt_tp()
            xpSerdesInt_t_Ptr.interrupt = int(args[3], 16)
            xpSerdesInt_t_Ptr.param = int(args[4], 16)
            xpSerdesInt_t_Ptr.ret = int(args[5], 16)
            xpSerdesInt_t_Ptr.flags = eval(args[6])
            #print('Input Arguments are, devId=%d, serdesId=%d, numElements=%d, interrupt=0x%x, param=0x%x, ret=0x%x, flags=%d' % (args[0],args[1],args[2],xpSerdesInt_t_Ptr.interrupt,xpSerdesInt_t_Ptr.param,xpSerdesInt_t_Ptr.ret,xpSerdesInt_t_Ptr.flags))
            ret = xpsSerdesCtrlrIntArray(args[0],args[1],args[2],xpSerdesInt_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('interrupt = %d' % (xpSerdesInt_t_Ptr.interrupt))
                print('param = %d' % (xpSerdesInt_t_Ptr.param))
                print('ret = %d' % (xpSerdesInt_t_Ptr.ret))
                print('flags = %d' % (xpSerdesInt_t_Ptr.flags))
                pass
            delete_xpSerdesInt_tp(xpSerdesInt_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrCrc
    #/********************************************************************************/
    def do_serdes_ctrlr_crc(self, arg):
        '''
         xpsSerdesCtrlrCrc: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            crcResult_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesCtrlrCrc(args[0],args[1],crcResult_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('crcResult = %d' % (uint32_tp_value(crcResult_Ptr_2)))
                pass
            delete_uint32_tp(crcResult_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrRamBist
    #/********************************************************************************/
    def do_serdes_ctrlr_ram_bist(self, arg):
        '''
         xpsSerdesCtrlrRamBist: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesCtrlrRamBist(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesDfeRunning
    #/********************************************************************************/
    def do_serdes_dfe_running(self, arg):
        '''
         xpsSerdesDfeRunning: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dfeRunning_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesDfeRunning(args[0],args[1],dfeRunning_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('dfeRunning = %d' % (uint8_tp_value(dfeRunning_Ptr_2)))
                pass
            delete_uint8_tp(dfeRunning_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetSignalOkEnable
    #/********************************************************************************/
    def do_serdes_get_signal_ok_enable(self, arg):
        '''
         xpsSerdesGetSignalOkEnable: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            signalOkEnable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetSignalOkEnable(args[0],args[1],signalOkEnable_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('signalOkEnable = %d' % (uint8_tp_value(signalOkEnable_Ptr_2)))
                pass
            delete_uint8_tp(signalOkEnable_Ptr_2)

    #/********************************************************************************/
    # command for xpsSerdesGetSignalOkThreshold
    #/********************************************************************************/
    def do_serdes_get_signal_ok_threshold(self, arg):
        '''
         xpsSerdesGetSignalOkThreshold: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetSignalOkThreshold(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Threshold Value = %d' % (ret))
                pass

    #/********************************************************************************/
    # command for xpsSerdesEyeGet
    #/********************************************************************************/
    def do_serdes_eye_get(self, arg):
        '''
         xpsSerdesEyeGet: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesEyeGet(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesSetAsyncCancelFlag
    #/********************************************************************************/
    def do_serdes_set_async_cancel_flag(self, arg):
        '''
         xpsSerdesSetAsyncCancelFlag: Enter [ devId,serdesId,newValue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,newValue ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2], 16)
            #print('Input Arguments are, devId=%d, serdesId=%d, newValue=0x%x' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetAsyncCancelFlag(args[0],args[1],args[2])
            if ret < 0:
                print('Error returned = %d' % (ret))
            else:
                print('Previous Value = %d' % (ret))
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetFirmwareBuild
    #/********************************************************************************/
    def do_serdes_get_firmware_build(self, arg):
        '''
         xpsSerdesGetFirmwareBuild: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetFirmwareBuild(args[0],args[1])
            if ret < 0:
                print('Error returned = %d' % (ret))
            else:
                print('Firmware Build = %d' % (ret))
                pass

    #/********************************************************************************/
    # command for xpsSerdesAddrInitBroadcast
    #/********************************************************************************/
    def do_serdes_addr_init_broadcast(self, arg):
        '''
         xpsSerdesAddrInitBroadcast: Enter [ devId,serdesId,chip,ring,sbus,lane ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,chip,ring,sbus,lane ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesAddr_t_Ptr = new_xpSerdesAddr_tp()
            xpSerdesAddr_t_Ptr.chip = int(args[2])
            xpSerdesAddr_t_Ptr.ring = int(args[3])
            xpSerdesAddr_t_Ptr.sbus = int(args[4])
            xpSerdesAddr_t_Ptr.lane = int(args[5])
            xpSerdesAddr_t_Ptr.next = int(args[6])
            serdesStatus_Ptr_7 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, chip=%d, ring=%d, sbus=%d, lane=%d' % (args[0],args[1],xpSerdesAddr_t_Ptr.chip,xpSerdesAddr_t_Ptr.ring,xpSerdesAddr_t_Ptr.sbus,xpSerdesAddr_t_Ptr.lane,xpSerdesAddr_t_Ptr.next))
            ret = xpsSerdesAddrInitBroadcast(args[0],args[1],xpSerdesAddr_t_Ptr,serdesStatus_Ptr_7)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('chip = %d' % (xpSerdesAddr_t_Ptr.chip))
                print('ring = %d' % (xpSerdesAddr_t_Ptr.ring))
                print('sbus = %d' % (xpSerdesAddr_t_Ptr.sbus))
                print('lane = %d' % (xpSerdesAddr_t_Ptr.lane))
                print('next = %d' % (xpSerdesAddr_t_Ptr.next))
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_7)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_7)
            delete_xpSerdesAddr_tp(xpSerdesAddr_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesDeviceInfo
    #/********************************************************************************/
    def do_serdes_device_info(self, arg):
        '''
         xpsSerdesDeviceInfo: Enter [ devId,serdesId,chip,ring,sbus,lane,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,chip,ring,sbus,lane,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesAddr_t_Ptr = new_xpSerdesAddr_tp()
            xpSerdesAddr_t_Ptr.chip = int(args[2])
            xpSerdesAddr_t_Ptr.ring = int(args[3])
            xpSerdesAddr_t_Ptr.sbus = int(args[4])
            xpSerdesAddr_t_Ptr.lane = int(args[5])
            xpSerdesAddr_t_Ptr.next = int(args[6])
            args[7] = eval(args[7])
            #print('Input Arguments are, devId=%d, serdesId=%d, chip=%d, ring=%d, sbus=%d, lane=%d, type=%d' % (args[0],args[1],xpSerdesAddr_t_Ptr.chip,xpSerdesAddr_t_Ptr.ring,xpSerdesAddr_t_Ptr.sbus,xpSerdesAddr_t_Ptr.lane,xpSerdesAddr_t_Ptr.next,args[7]))
            ret = xpsSerdesDeviceInfo(args[0],args[1],xpSerdesAddr_t_Ptr,args[7])
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('chip = %d' % (xpSerdesAddr_t_Ptr.chip))
                print('ring = %d' % (xpSerdesAddr_t_Ptr.ring))
                print('sbus = %d' % (xpSerdesAddr_t_Ptr.sbus))
                print('lane = %d' % (xpSerdesAddr_t_Ptr.lane))
                print('next = %d' % (xpSerdesAddr_t_Ptr.next))
                pass
            delete_xpSerdesAddr_tp(xpSerdesAddr_t_Ptr)

    #/********************************************************************************/
    # command for xpsSerdesSerdesWrFlush
    #/********************************************************************************/
    def do_serdes_serdes_wr_flush(self, arg):
        '''
         xpsSerdesSerdesWrFlush: Enter [ devId,serdesId,reg,data ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,reg,data ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3], 16)
            serdesStatus_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, reg=%d, data=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesSerdesWrFlush(args[0],args[1],args[2],args[3],serdesStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_4)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesSetTxDataSel
    #/********************************************************************************/
    def do_serdes_set_tx_data_sel(self, arg):
        '''
         xpsSerdesSetTxDataSel: Enter [ devId,serdesId,input ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,input ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, input=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSetTxDataSel(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetTxLineEncoding
    #/********************************************************************************/
    def do_serdes_get_tx_line_encoding(self, arg):
        '''
         xpsSerdesGetTxLineEncoding: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            serdesStatus_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetTxLineEncoding(args[0],args[1],serdesStatus_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_3)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesGetRxLineEncoding
    #/********************************************************************************/
    def do_serdes_get_rx_line_encoding(self, arg):
        '''
         xpsSerdesGetRxLineEncoding: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            serdesStatus_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))

            ret = xpsSerdesGetRxLineEncoding(args[0],args[1],serdesStatus_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_3)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesSlipBits
    #/********************************************************************************/
    def do_serdes_slip_bits(self, arg):
        '''
         xpsSerdesSlipBits: Enter [ devId,serdesId,bits ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,bits ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            serdesStatus_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, bits=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesSlipBits(args[0],args[1],args[2],serdesStatus_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_3)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesSlipRxPhase
    #/********************************************************************************/
    def do_serdes_slip_rx_phase(self, arg):
        '''
         xpsSerdesSlipRxPhase: Enter [ devId,serdesId,bits,applyAtInit ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,bits,applyAtInit ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            serdesStatus_Ptr_4 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, bits=%d, applyAtInit=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesSlipRxPhase(args[0],args[1],args[2],args[3],serdesStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_4)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesSlipTxPhase
    #/********************************************************************************/
    def do_serdes_slip_tx_phase(self, arg):
        '''
         xpsSerdesSlipTxPhase: Enter [ devId,serdesId,bits,applyAtInit ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,bits,applyAtInit ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            serdesStatus_Ptr_4 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, bits=%d, applyAtInit=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesSlipTxPhase(args[0],args[1],args[2],args[3],serdesStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_4)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesTuneInit
    #/********************************************************************************/
    def do_serdes_tune_init(self, arg):
        '''
         xpsSerdesTuneInit: Enter [ devId,serdesId,fixed,value,dfeDisable,tuneMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,fixed,value,dfeDisable,tuneMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdesDfeTune_t_Ptr = new_xpSerdesDfeTune_tp()

            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdesDfeTune_t_Ptr.fixed[ix] = int(postList[ix])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdesDfeTune_t_Ptr.value[ix] = int(postList[ix])
            xpSerdesDfeTune_t_Ptr.dfeDisable = int(args[4])
            xpSerdesDfeTune_t_Ptr.tuneMode = eval(args[5])
            #print('Input Arguments are, devId=%d, serdesId=%d, fixed=%s, value=%s, dfeDisable=%d, tuneMode=%d' % (args[0],args[1],args[2],args[3],xpSerdesDfeTune_t_Ptr.dfeDisable,xpSerdesDfeTune_t_Ptr.tuneMode))
            ret = xpsSerdesTuneInit(args[0],args[1],xpSerdesDfeTune_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('fixed = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdesDfeTune_t_Ptr.fixed[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('value = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdesDfeTune_t_Ptr.value[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('dfeDisable = %d' % (xpSerdesDfeTune_t_Ptr.dfeDisable))
                print('tuneMode = %d' % (xpSerdesDfeTune_t_Ptr.tuneMode))
                pass
            delete_xpSerdesDfeTune_tp(xpSerdesDfeTune_t_Ptr)
            delete_arrUint32(xpSerdesDfeTune_t_Ptr.fixed)

    #/********************************************************************************/
    # command for xpsSerdesEyeGetSimpleMetric
    #/********************************************************************************/
    def do_serdes_eye_get_simple_metric(self, arg):
        '''
         xpsSerdesEyeGetSimpleMetric: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeMetric_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesEyeGetSimpleMetric(args[0],args[1],eyeMetric_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('eyeMetric = %d' % (uint32_tp_value(eyeMetric_Ptr_3)))
                pass
            delete_uint32_tp(eyeMetric_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesGetRxLiveData
    #/********************************************************************************/
    def do_serdes_get_rx_live_data(self, arg):
        '''
         xpsSerdesGetRxLiveData: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetRxLiveData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesCtrlrDiag
    #/********************************************************************************/
    def do_serdes_ctrlr_diag(self, arg):
        '''
         xpsSerdesCtrlrDiag: Enter [ devId,serdesId,cycles ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,cycles ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            serdesStatus_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, cycles=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesCtrlrDiag(args[0],args[1],args[2],serdesStatus_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint32_tp_value(serdesStatus_Ptr_4)))
                pass
            delete_uint32_tp(serdesStatus_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesFirmwareUpload_2
    #/********************************************************************************/
    def do_serdes_firmware_upload_2(self, arg):
        '''
         xpsSerdesFirmwareUpload_2: Enter [ devId,serdesId,serdesRomSize,serdesCtrlrRomSize,sdiRomSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,serdesRomSize,serdesCtrlrRomSize,sdiRomSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            serdesRom_Ptr_4 = new_int32_tp()
            args[3] = int(args[3])
            serdesCtrlrRom_Ptr_6 = new_int32_tp()
            args[4] = int(args[4])
            sdiRom_Ptr_8 = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, serdesRomSize=%d, serdesCtrlrRomSize=%d, sdiRomSize=%d' % (args[0],args[1],args[2],args[3],args[5]))
            ret = xpsSerdesFirmwareUpload_2(args[0],args[1],args[2],serdesRom_Ptr_4,args[3],serdesCtrlrRom_Ptr_6,args[4],sdiRom_Ptr_8)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesRom = %d' % (int32_tp_value(serdesRom_Ptr_4)))
                print('serdesCtrlrRom = %d' % (int32_tp_value(serdesCtrlrRom_Ptr_6)))
                print('sdiRom = %d' % (int32_tp_value(sdiRom_Ptr_8)))
                pass
            delete_int32_tp(sdiRom_Ptr_8)
            delete_int32_tp(serdesCtrlrRom_Ptr_6)
            delete_int32_tp(serdesRom_Ptr_4)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrWaitForUpload
    #/********************************************************************************/
    def do_serdes_ctrlr_wait_for_upload(self, arg):
        '''
         xpsSerdesCtrlrWaitForUpload: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesCtrlrWaitForUpload(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesTwiWaitForComplete
    #/********************************************************************************/
    def do_serdes_twi_wait_for_complete(self, arg):
        '''
         xpsSerdesTwiWaitForComplete: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesTwiWaitForComplete(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSerdesGetSerdesCtleVal
    #/********************************************************************************/
    def do_get_serdes_ctle_val(self, arg):
        '''
         xpsSerdesGetSerdesCtleVal: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            ctleVal = new_xpSerdesCtle_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetSerdesCtleVal(args[0],serdesId, ctleVal)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('HF = %d, LF = %d, DC = %d, BW = %d for serdesId = %d' % (ctleVal.hf,ctleVal.lf,ctleVal.dc,ctleVal.bw,serdesId))
                    pass


    #/********************************************************************************/
    # command for xpsSerdesFirmwareUploadFile_2
    #/********************************************************************************/
    def do_serdes_firmware_upload_file_2(self, arg):
        '''
         xpsSerdesFirmwareUploadFile_2: Enter [ devId,serdesId,serdesRomFile,serdesCtrlrRomFile,sdiRomFile ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,serdesRomFile,serdesCtrlrRomFile,sdiRomFile ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            serdesRomFile_Ptr_3 = str(args[2]) 
            serdesCtrlrRomFile_Ptr_4 = str(args[3])
            sdiRomFile_Ptr_5 = str(args[4])
            #print('Input Arguments are, devId=%d, serdesId=%d, serdesRomFile=%s, serdesCtrlrRomFile=%s, sdiRomFile=%s' % (args[0],args[1],serdesRomFile,serdesCtrlrRomFile,sdiRomFile))
            ret = xpsSerdesFirmwareUploadFile_2(args[0],args[1],serdesRomFile_Ptr_3,serdesCtrlrRomFile_Ptr_4,sdiRomFile_Ptr_5)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesRomFile = %s' % serdesRomFile_Ptr_3)
                print('serdesCtrlrRomFile = %s' % serdesCtrlrRomFile_Ptr_4)
                print('sdiRomFile = %s' % sdiRomFile_Ptr_5)
                pass

    #/********************************************************************************/
    # command for xpsSerdesCtrlrIntCheck
    #/********************************************************************************/
    def do_serdes_ctrlr_int_check(self, arg):
        '''
         xpsSerdesCtrlrIntCheck: Enter [ devId,serdesId,intNum,param ]
        '''
        info=inspect.currentframe()
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,intNum,param ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            serdesStatus_Ptr_7 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, intNum=%d, param=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSerdesCtrlrIntCheck(args[0],args[1],info.f_code.co_name,info.f_lineno,args[2],args[3],serdesStatus_Ptr_7)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('serdesStatus = %d' % (uint8_tp_value(serdesStatus_Ptr_7)))
                pass
            delete_uint8_tp(serdesStatus_Ptr_7)

    #/********************************************************************************/
    # command for xpsSerdesSetStruct
    #/********************************************************************************/
    def do_serdes_set_struct(self, arg):
        '''
         xpsSerdesSetStruct: Enter [ devId,serdesId,debug,suppressErrors,upgradeWarnings,enableDebugLogging,enableStreamLogging,enableStreamErrLogging,logTimeStamps,serdesIntTimeout,sbusMdioTimeout,enableSerdesCorePortInterrupt,chips,maxCmdsBuffered,priorConnection,cmdsBuffered,socket,tcpPort,jtagIdcode,processId,lastMdioAddr,mdioBasePortAddr,i2cBaseAddr,ipType,lsbRev,maxSbusAddr,returnCode,logSize,data,dataCharSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 30
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,debug,suppressErrors,upgradeWarnings,enableDebugLogging,enableStreamLogging,enableStreamErrLogging,logTimeStamps,serdesIntTimeout,sbusMdioTimeout,enableSerdesCorePortInterrupt,chips,maxCmdsBuffered,priorConnection,cmdsBuffered,socket,tcpPort,jtagIdcode,processId,lastMdioAddr,mdioBasePortAddr,i2cBaseAddr,ipType,lsbRev,maxSbusAddr,returnCode,logSize,data,dataCharSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdes_t_Ptr = xpSerdes_t()
            xpSerdes_t_Ptr.debug = int(args[2])
            xpSerdes_t_Ptr.suppressErrors = int(args[3])
            xpSerdes_t_Ptr.upgradeWarnings = int(args[4])
            xpSerdes_t_Ptr.enableDebugLogging = int(args[5])
            xpSerdes_t_Ptr.enableStreamLogging = int(args[6])
            xpSerdes_t_Ptr.enableStreamErrLogging = int(args[7])
            xpSerdes_t_Ptr.logTimeStamps = int(args[8])
            xpSerdes_t_Ptr.serdesIntTimeout = int(args[9])
            xpSerdes_t_Ptr.sbusMdioTimeout = int(args[10])
            xpSerdes_t_Ptr.enableSerdesCorePortInterrupt = int(args[11])
            xpSerdes_t_Ptr.chips = int(args[12])
            xpSerdes_t_Ptr.maxCmdsBuffered = int(args[13])
            xpSerdes_t_Ptr.priorConnection = int(args[14])
            xpSerdes_t_Ptr.cmdsBuffered = int(args[15])
            xpSerdes_t_Ptr.bufCmd = int(args[16])

            args[17] = args[17].replace(".",":").replace(",",":")
            postList = args[17].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            xpSerdes_t_Ptr.chipName = new_arrUint32(listLen+1)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.chipName[ix] = int(postList[ix])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            xpSerdes_t_Ptr.chipRev = new_arrUint32(listLen+1)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.chipRev[ix] = int(postList[ix])
            xpSerdes_t_Ptr.aacsServer = int(args[19])
            xpSerdes_t_Ptr.socket = int(args[20])
            xpSerdes_t_Ptr.tcpPort = int(args[21])

            args[22] = args[22].replace(".",":").replace(",",":")
            postList = args[22].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.jtagIdcode[ix] = int(postList[ix])

            args[23] = args[23].replace(".",":").replace(",",":")
            postList = args[23].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.processId[ix] = int(postList[ix])

            args[24] = args[24].replace(".",":").replace(",",":")
            postList = args[24].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.lastMdioAddr[ix] = int(postList[ix])
            xpSerdes_t_Ptr.mdioBasePortAddr = int(args[25], 16)
            xpSerdes_t_Ptr.i2cBaseAddr = int(args[26], 16)

            args[27] = args[27].replace(".",":").replace(",",":")
            postList = args[27].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.ipType[ix] = int(postList[ix])

            args[28] = args[28].replace(".",":").replace(",",":")
            postList = args[28].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.lsbRev[ix] = int(postList[ix])

            args[29] = args[29].replace(".",":").replace(",",":")
            postList = args[29].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.maxSbusAddr[ix] = int(postList[ix])
            xpSerdes_t_Ptr.returnCode = int(args[30])
            xpSerdes_t_Ptr.logSize = int(args[31])
            xpSerdes_t_Ptr.data = int(args[32], 16)
            xpSerdes_t_Ptr.dataCharSize = int(args[33], 16)
            xpSerdes_t_Ptr.dataChar = int(args[34])
            xpSerdes_t_Ptr.dataCharEnd = int(args[35])
            xpSerdes_t_Ptr.clientData = int(args[36])
            xpSerdes_t_Ptr.log = int(args[37])
            xpSerdes_t_Ptr.logEnd = int(args[38])
            #print('Input Arguments are, devId=%d, serdesId=%d, debug=%d, suppressErrors=%d, upgradeWarnings=%d, enableDebugLogging=%d, enableStreamLogging=%d, enableStreamErrLogging=%d, logTimeStamps=%d, serdesIntTimeout=%d, sbusMdioTimeout=%d, enableSerdesCorePortInterrupt=%d, chips=%d, maxCmdsBuffered=%d, priorConnection=%d, cmdsBuffered=%d, socket=%d, tcpPort=%d, jtagIdcode=%s, processId=%s, lastMdioAddr=%s, mdioBasePortAddr=0x%x, i2cBaseAddr=0x%x, ipType=%s, lsbRev=%s, maxSbusAddr=%s, returnCode=%d, logSize=%d, data=0x%x, dataCharSize=0x%x' % (args[0],args[1],xpSerdes_t_Ptr.debug,xpSerdes_t_Ptr.suppressErrors,xpSerdes_t_Ptr.upgradeWarnings,xpSerdes_t_Ptr.enableDebugLogging,xpSerdes_t_Ptr.enableStreamLogging,xpSerdes_t_Ptr.enableStreamErrLogging,xpSerdes_t_Ptr.logTimeStamps,xpSerdes_t_Ptr.serdesIntTimeout,xpSerdes_t_Ptr.sbusMdioTimeout,xpSerdes_t_Ptr.enableSerdesCorePortInterrupt,xpSerdes_t_Ptr.chips,xpSerdes_t_Ptr.maxCmdsBuffered,xpSerdes_t_Ptr.priorConnection,xpSerdes_t_Ptr.cmdsBuffered,xpSerdes_t_Ptr.bufCmd,args[17],args[18],xpSerdes_t_Ptr.aacsServer,xpSerdes_t_Ptr.socket,xpSerdes_t_Ptr.tcpPort,args[22],args[23],args[24],xpSerdes_t_Ptr.mdioBasePortAddr,xpSerdes_t_Ptr.i2cBaseAddr,args[27],args[28],args[29],xpSerdes_t_Ptr.returnCode,xpSerdes_t_Ptr.logSize,xpSerdes_t_Ptr.data,xpSerdes_t_Ptr.dataCharSize,xpSerdes_t_Ptr.dataChar,xpSerdes_t_Ptr.dataCharEnd,xpSerdes_t_Ptr.clientData,xpSerdes_t_Ptr.log,xpSerdes_t_Ptr.logEnd))
            ret = xpsSerdesSetStruct(args[0],args[1],xpSerdes_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('bufCmd = %d' % (xpSerdes_t_Ptr.bufCmd))
                print('chipName = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.chipName[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('chipRev = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.chipRev[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('aacsServer = %d' % (xpSerdes_t_Ptr.aacsServer))
                print('socket = %d' % (xpSerdes_t_Ptr.socket))
                print('tcpPort = %d' % (xpSerdes_t_Ptr.tcpPort))
                print('jtagIdcode = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.jtagIdcode[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('processId = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.processId[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('lastMdioAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.lastMdioAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('mdioBasePortAddr = %d' % (xpSerdes_t_Ptr.mdioBasePortAddr))
                print('i2cBaseAddr = %d' % (xpSerdes_t_Ptr.i2cBaseAddr))
                print('ipType = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.ipType[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('lsbRev = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.lsbRev[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('maxSbusAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.maxSbusAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('returnCode = %d' % (xpSerdes_t_Ptr.returnCode))
                print('logSize = %d' % (xpSerdes_t_Ptr.logSize))
                print('data = %d' % (xpSerdes_t_Ptr.data))
                print('dataCharSize = %d' % (xpSerdes_t_Ptr.dataCharSize))
                print('dataChar = %d' % (xpSerdes_t_Ptr.dataChar))
                print('dataCharEnd = %d' % (xpSerdes_t_Ptr.dataCharEnd))
                print('clientData = %d' % (xpSerdes_t_Ptr.clientData))
                print('log = %d' % (xpSerdes_t_Ptr.log))
                print('logEnd = %d' % (xpSerdes_t_Ptr.logEnd))
                pass
            delete_arrUint32(xpSerdes_t_Ptr.chipRev)
            delete_arrUint32(xpSerdes_t_Ptr.chipName)

    #/********************************************************************************/
    # command for xpsSerdesGetStruct
    #/********************************************************************************/
    def do_serdes_get_struct(self, arg):
        '''
         xpsSerdesGetStruct: Enter [ devId,serdesId,debug,suppressErrors,upgradeWarnings,enableDebugLogging,enableStreamLogging,enableStreamErrLogging,logTimeStamps,serdesIntTimeout,sbusMdioTimeout,enableSerdesCorePortInterrupt,chips,maxCmdsBuffered,priorConnection,cmdsBuffered,socket,tcpPort,jtagIdcode,processId,lastMdioAddr,mdioBasePortAddr,i2cBaseAddr,ipType,lsbRev,maxSbusAddr,returnCode,logSize,data,dataCharSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 30
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,debug,suppressErrors,upgradeWarnings,enableDebugLogging,enableStreamLogging,enableStreamErrLogging,logTimeStamps,serdesIntTimeout,sbusMdioTimeout,enableSerdesCorePortInterrupt,chips,maxCmdsBuffered,priorConnection,cmdsBuffered,socket,tcpPort,jtagIdcode,processId,lastMdioAddr,mdioBasePortAddr,i2cBaseAddr,ipType,lsbRev,maxSbusAddr,returnCode,logSize,data,dataCharSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpSerdes_t_Ptr = new_xpSerdes_tp()
            xpSerdes_t_Ptr.debug = int(args[2])
            xpSerdes_t_Ptr.suppressErrors = int(args[3])
            xpSerdes_t_Ptr.upgradeWarnings = int(args[4])
            xpSerdes_t_Ptr.enableDebugLogging = int(args[5])
            xpSerdes_t_Ptr.enableStreamLogging = int(args[6])
            xpSerdes_t_Ptr.enableStreamErrLogging = int(args[7])
            xpSerdes_t_Ptr.logTimeStamps = int(args[8])
            xpSerdes_t_Ptr.serdesIntTimeout = int(args[9])
            xpSerdes_t_Ptr.sbusMdioTimeout = int(args[10])
            xpSerdes_t_Ptr.enableSerdesCorePortInterrupt = int(args[11])
            xpSerdes_t_Ptr.chips = int(args[12])
            xpSerdes_t_Ptr.maxCmdsBuffered = int(args[13])
            xpSerdes_t_Ptr.priorConnection = int(args[14])
            xpSerdes_t_Ptr.cmdsBuffered = int(args[15])
            xpSerdes_t_Ptr.bufCmd = int(args[16])

            args[17] = args[17].replace(".",":").replace(",",":")
            postList = args[17].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            xpSerdes_t_Ptr.chipName = new_arrUint32(listLen+1)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.chipName[ix] = int(postList[ix])

            args[18] = args[18].replace(".",":").replace(",",":")
            postList = args[18].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            xpSerdes_t_Ptr.chipRev = new_arrUint32(listLen+1)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.chipRev[ix] = int(postList[ix])
            xpSerdes_t_Ptr.aacsServer = int(args[19])
            xpSerdes_t_Ptr.socket = int(args[20])
            xpSerdes_t_Ptr.tcpPort = int(args[21])

            args[22] = args[22].replace(".",":").replace(",",":")
            postList = args[22].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.jtagIdcode[ix] = int(postList[ix])

            args[23] = args[23].replace(".",":").replace(",",":")
            postList = args[23].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.processId[ix] = int(postList[ix])

            args[24] = args[24].replace(".",":").replace(",",":")
            postList = args[24].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.lastMdioAddr[ix] = int(postList[ix])
            xpSerdes_t_Ptr.mdioBasePortAddr = int(args[25], 16)
            xpSerdes_t_Ptr.i2cBaseAddr = int(args[26], 16)

            args[27] = args[27].replace(".",":").replace(",",":")
            postList = args[27].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.ipType[ix] = int(postList[ix])

            args[28] = args[28].replace(".",":").replace(",",":")
            postList = args[28].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.lsbRev[ix] = int(postList[ix])

            args[29] = args[29].replace(".",":").replace(",",":")
            postList = args[29].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen):
                xpSerdes_t_Ptr.maxSbusAddr[ix] = int(postList[ix])
            xpSerdes_t_Ptr.returnCode = int(args[30])
            xpSerdes_t_Ptr.logSize = int(args[31])
            xpSerdes_t_Ptr.data = int(args[32], 16)
            xpSerdes_t_Ptr.dataCharSize = int(args[33], 16)
            xpSerdes_t_Ptr.dataChar = int(args[34])
            xpSerdes_t_Ptr.dataCharEnd = int(args[35])
            xpSerdes_t_Ptr.clientData = int(args[36])
            xpSerdes_t_Ptr.log = int(args[37])
            xpSerdes_t_Ptr.logEnd = int(args[38])
            #print('Input Arguments are, devId=%d, serdesId=%d, debug=%d, suppressErrors=%d, upgradeWarnings=%d, enableDebugLogging=%d, enableStreamLogging=%d, enableStreamErrLogging=%d, logTimeStamps=%d, serdesIntTimeout=%d, sbusMdioTimeout=%d, enableSerdesCorePortInterrupt=%d, chips=%d, maxCmdsBuffered=%d, priorConnection=%d, cmdsBuffered=%d, socket=%d, tcpPort=%d, jtagIdcode=%s, processId=%s, lastMdioAddr=%s, mdioBasePortAddr=0x%x, i2cBaseAddr=0x%x, ipType=%s, lsbRev=%s, maxSbusAddr=%s, returnCode=%d, logSize=%d, data=0x%x, dataCharSize=0x%x' % (args[0],args[1],xpSerdes_t_Ptr.debug,xpSerdes_t_Ptr.suppressErrors,xpSerdes_t_Ptr.upgradeWarnings,xpSerdes_t_Ptr.enableDebugLogging,xpSerdes_t_Ptr.enableStreamLogging,xpSerdes_t_Ptr.enableStreamErrLogging,xpSerdes_t_Ptr.logTimeStamps,xpSerdes_t_Ptr.serdesIntTimeout,xpSerdes_t_Ptr.sbusMdioTimeout,xpSerdes_t_Ptr.enableSerdesCorePortInterrupt,xpSerdes_t_Ptr.chips,xpSerdes_t_Ptr.maxCmdsBuffered,xpSerdes_t_Ptr.priorConnection,xpSerdes_t_Ptr.cmdsBuffered,xpSerdes_t_Ptr.bufCmd,args[17],args[18],xpSerdes_t_Ptr.aacsServer,xpSerdes_t_Ptr.socket,xpSerdes_t_Ptr.tcpPort,args[22],args[23],args[24],xpSerdes_t_Ptr.mdioBasePortAddr,xpSerdes_t_Ptr.i2cBaseAddr,args[27],args[28],args[29],xpSerdes_t_Ptr.returnCode,xpSerdes_t_Ptr.logSize,xpSerdes_t_Ptr.data,xpSerdes_t_Ptr.dataCharSize,xpSerdes_t_Ptr.dataChar,xpSerdes_t_Ptr.dataCharEnd,xpSerdes_t_Ptr.clientData,xpSerdes_t_Ptr.log,xpSerdes_t_Ptr.logEnd))
            ret = xpsSerdesGetStruct(args[0],args[1],xpSerdes_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('debug = %d' % (xpSerdes_t_Ptr.debug))
                print('suppressErrors = %d' % (xpSerdes_t_Ptr.suppressErrors))
                print('upgradeWarnings = %d' % (xpSerdes_t_Ptr.upgradeWarnings))
                print('enableDebugLogging = %d' % (xpSerdes_t_Ptr.enableDebugLogging))
                print('enableStreamLogging = %d' % (xpSerdes_t_Ptr.enableStreamLogging))
                print('enableStreamErrLogging = %d' % (xpSerdes_t_Ptr.enableStreamErrLogging))
                print('logTimeStamps = %d' % (xpSerdes_t_Ptr.logTimeStamps))
                print('serdesIntTimeout = %d' % (xpSerdes_t_Ptr.serdesIntTimeout))
                print('sbusMdioTimeout = %d' % (xpSerdes_t_Ptr.sbusMdioTimeout))
                print('enableSerdesCorePortInterrupt = %d' % (xpSerdes_t_Ptr.enableSerdesCorePortInterrupt))
                print('chips = %d' % (xpSerdes_t_Ptr.chips))
                print('maxCmdsBuffered = %d' % (xpSerdes_t_Ptr.maxCmdsBuffered))
                print('priorConnection = %d' % (xpSerdes_t_Ptr.priorConnection))
                print('cmdsBuffered = %d' % (xpSerdes_t_Ptr.cmdsBuffered))
                print('bufCmd = %d' % (xpSerdes_t_Ptr.bufCmd))
                print('chipName = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.chipName[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('chipRev = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.chipRev[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('aacsServer = %d' % (xpSerdes_t_Ptr.aacsServer))
                print('socket = %d' % (xpSerdes_t_Ptr.socket))
                print('tcpPort = %d' % (xpSerdes_t_Ptr.tcpPort))
                print('jtagIdcode = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.jtagIdcode[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('processId = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.processId[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('lastMdioAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.lastMdioAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('mdioBasePortAddr = %d' % (xpSerdes_t_Ptr.mdioBasePortAddr))
                print('i2cBaseAddr = %d' % (xpSerdes_t_Ptr.i2cBaseAddr))
                print('ipType = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.ipType[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('lsbRev = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.lsbRev[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('maxSbusAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpSerdes_t_Ptr.maxSbusAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('returnCode = %d' % (xpSerdes_t_Ptr.returnCode))
                print('logSize = %d' % (xpSerdes_t_Ptr.logSize))
                print('data = %d' % (xpSerdes_t_Ptr.data))
                print('dataCharSize = %d' % (xpSerdes_t_Ptr.dataCharSize))
                print('dataChar = %d' % (xpSerdes_t_Ptr.dataChar))
                print('dataCharEnd = %d' % (xpSerdes_t_Ptr.dataCharEnd))
                print('clientData = %d' % (xpSerdes_t_Ptr.clientData))
                print('log = %d' % (xpSerdes_t_Ptr.log))
                print('logEnd = %d' % (xpSerdes_t_Ptr.logEnd))
                pass
            delete_arrUint32(xpSerdes_t_Ptr.chipRev)
            delete_arrUint32(xpSerdes_t_Ptr.chipName)
            delete_xpSerdes_tp(xpSerdes_t_Ptr)


    #/********************************************************************************/
    # command for xpsSerdesGetEyeMeasurement
    #/********************************************************************************/
    def do_serdes_get_eye_measurement(self, arg):
        '''
         xpsSerdesGetEyeMeasurement: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeHeight_Ptr_2 = new_uint32_tp()
            eyeWidth_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetEyeMeasurement(args[0],args[1], eyeHeight_Ptr_2, eyeWidth_Ptr_3)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('eyeHeight = %d' % (uint32_tp_value(eyeHeight_Ptr_2)))
                print('eyeWidth = %d' % (uint32_tp_value(eyeWidth_Ptr_3)))
                pass
            delete_uint32_tp(eyeHeight_Ptr_2)
            delete_uint32_tp(eyeWidth_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesGetEyeMeasurementWithCustomConfig
    #/********************************************************************************/
    def do_serdes_get_eye_measurement_custom(self, arg):
        '''
         xpsSerdesGetEyeMeasurementWithCustomConfig: Enter [ devId,serdesId, [eye configuration parameters(optional)] ]
         Optional args are:\n\
         [\n\
             -eyeType [By default: EYE_FULL, valid range is <EYE_SIZE = 0, EYE_CROSS = 0, EYE_SIZE_VDUAL = 1, EYE_HEIGHT = 2, EYE_WIDTH = 3, EYE_FULL = 4, EYE_HEIGHT_DVOS = 5>]\n\
             -cmpMode [By default: 0x0100, valid values are <0x0000, 0x0100, 0x0120, 0x0200>]\n\
             -minDwellBits [Minimum samples to measure. By default: 500000]\n\
             -maxDwellBits [Maximum samples to measure. By default: 100000000]\n\
             -fastDynamic [By default: 2, valid range is <0, 1, 2>]\n\
             -xUI [By default: 1, valid range is <1, 2, 3, 4, 5, 6, 7, 8>]\n\
             -errorThreshold [By default: 30]\n\
             -dcBalance [By default: 0.5]\n\
             -transDensity [By default: 0.5]\n\
             -xResolution [Number of points per UI to gather. The requested value will be rounded down to the next lower power of 2, with a minimum of 8 points per UI. By default: 64]\n\
             -yCenterPoint [Center around which yPoints is gathered. By default: 0]\n\
             -yPoints [Number of vertical points to gather. By default: 129]\n\
             -yStepSize [Vertical step size. By default: 1]\n\
             -xAutoScale [By default: 1]\n\
             -yAutoScale [By default: 1]\n\
             -xShift [PI steps to adjust the eye center before gathering.]\n\
             -gatherMean [By default: 0, valid range is <FALSE = 0, TRUE = 1>] If -meanDirection is true, gather only the eye points that bracket the 50% error point.\n\
             -MeanThreshold [If ec_gather_mean is given, use this value as the 50% error point, overriding calculation.]\n\
             -noSbm [By default: 0, valid range is <FALSE = 0, TRUE = 1>] If -noSbm is true then sbus master assist will not be used even if avaiable.\n\
             -meanDirection [If -meanDirection is given, use this value to control direction.]\n\
         ]\n'
         Note: Without any optional arguments, command will be execute with default configuration values.
        '''

        args = arg.split()
        if  len(args) < 2:
            print 'Invalid input,\n\
                   \txpsSerdesGetEyeMeasurementWithCustomConfig: Enter [ devId,serdesId, [optional arguments] ]\n\
                   \tsee "help serdes_get_eye_measurement_custom" for optional arguments information.\n'
            return
        else:
            eyecfg = customPrepareEyeCfgData(args)
            if(eyecfg == None):
                return
            args[0] = int(args[0])
            args[1] = int(args[1])
            eyeHeight_Ptr_2 = new_uint32_tp()
            eyeWidth_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesGetEyeMeasurementWithCustomConfig(args[0], args[1], eyecfg, eyeHeight_Ptr_2, eyeWidth_Ptr_3)
            if ret != 0:
                print('Error: Return value = %d' % (ret))
            else:
                print('eyeHeight = %d' % (uint32_tp_value(eyeHeight_Ptr_2)))
                print('eyeWidth = %d' % (uint32_tp_value(eyeWidth_Ptr_3)))
                pass
            delete_uint32_tp(eyeHeight_Ptr_2)
            delete_uint32_tp(eyeWidth_Ptr_3)

    #/********************************************************************************/
    # command for xpsSerdesTimeoutSet
    #/********************************************************************************/
    def do_serdes_timeout_set(self, arg):
        '''
         xpsSerdesTimeoutSet: Enter [ devId, serdesId, timeoutVal ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, serdesId, timeoutVal]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, serdesId=%d, timeoutVal=%d' % (args[0],args[1],args[2]))
            ret = xpsSerdesTimeoutSet(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
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
    # command for xpsSerdesSensorGetTemperature
    #/********************************************************************************/
    def do_serdes_sensor_get_temperature(self, arg):
        '''
         xpsSerdesSensorGetTemperature: Enter [ devId,temperature ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            temp = new_int32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSerdesSensorGetTemperature(args[0], temp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Temperature is %d degree celsius' % (int32_tp_value(temp)/1000))
                pass
            delete_int32_tp(temp)

    #/********************************************************************************/
    # command for xpsSerdesCtrlrGetSerdesClockDivider
    #/********************************************************************************/
    def do_serdes_ctrlr_get_serdes_clock_divider(self, arg):
        '''
         xpsSerdesCtrlrGetSerdesClockDivider: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            divider = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesCtrlrGetSerdesClockDivider(args[0],serdesId, divider)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('divider = %d for serdesId = %d' % (int32_tp_value(divider),serdesId))
                    pass
            delete_int32_tp(divider)

    #/********************************************************************************/
    # command for xpsSerdesConnectionStatus
    #/********************************************************************************/
    def do_serdes_connection_status(self, arg):
        '''
         xpsSerdesConnectionStatus: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            status = new_int32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesConnectionStatus(args[0],serdesId, status)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('status = %d for serdesId = %d' % (int32_tp_value(status),serdesId))
                    pass
            delete_int32_tp(status)
    #/********************************************************************************/
    # command for xpsSerdesCloseConnection
    #/********************************************************************************/
    def do_serdes_close_connection(self, arg):
        '''
         xpsSerdesCloseConnection: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesCloseConnection(args[0],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesSetCtrlrRunningFlag
    #/********************************************************************************/
    def do_serdes_set_ctrlr_running_flag(self, arg):
        '''
         xpsSerdesSetCtrlrRunningFlag: Enter [ devId,serdesId/startSerdesId-endSerdesId,isSpicoRunning]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,isSpicoRunning]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d isSpicoRunning=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSetCtrlrRunningFlag(args[0],serdesId,int(args[2]))
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesGetCtrlrRunningFlag
    #/********************************************************************************/
    def do_serdes_get_ctrlr_running_flag(self, arg):
        '''
         xpsSerdesGetCtrlrRunningFlag: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            ctrlRunFlag = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetCtrlrRunningFlag(args[0],serdesId, ctrlRunFlag)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('ctrlRunFlag = %d for serdesId = %d' % (int32_tp_value(ctrlRunFlag),serdesId))

            delete_int32_tp(ctrlRunFlag)

    #/********************************************************************************/
    # command for xpsSerdesGetInterruptRev
    #/********************************************************************************/
    def do_serdes_get_interrupt_rev(self, arg):
        '''
         xpsSerdesGetInterruptRev: Enter [ devId,intNum,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intNum,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            if '-' in args[2]:
                startIdx = int(args[2].split('-')[0].strip())
                endIdx = int(args[2].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[2] = int(args[2])
                startIdx = args[2]
                endIdx = args[2]
            rev = new_int32_tp()
            #print('Input Arguments are, devId=%d, intNum=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetInterruptRev(args[0],args[1],serdesId, rev)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('rev = %d for serdesId = %d' % (int32_tp_value(rev),serdesId))
                    pass
            delete_int32_tp(rev)

    #/********************************************************************************/
    # command for xpsSerdesGetSdrev
    #/********************************************************************************/
    def do_serdes_get_sdrev(self, arg):
        '''
         xpsSerdesGetSdrev: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            rev = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetSdrev(args[0],serdesId, rev)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('SD rev = %d for serdesId = %d' % (int32_tp_value(rev),serdesId))

            delete_int32_tp(rev)

    #/********************************************************************************/
    # command for xpsSerdesInitQuick
    #/********************************************************************************/
    def do_serdes_init_quick(self, arg):
        '''
         xpsSerdesInitQuick: Enter [ devId,divider,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,divider,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            if '-' in args[2]:
                startIdx = int(args[2].split('-')[0].strip())
                endIdx = int(args[2].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[2] = int(args[2])
                startIdx = args[2]
                endIdx = args[2]
            #print('Input Arguments are, devId=%d, divider=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesInitQuick(args[0],args[1],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesResume
    #/********************************************************************************/
    def do_serdes_resume(self, arg):
        '''
         xpsSerdesResume: Enter [ devId,initialState,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initialState,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            if '-' in args[2]:
                startIdx = int(args[2].split('-')[0].strip())
                endIdx = int(args[2].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[2] = int(args[2])
                startIdx = args[2]
                endIdx = args[2]
            #print('Input Arguments are, devId=%d, initialState=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesResume(args[0],args[1],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesHalt
    #/********************************************************************************/
    def do_serdes_halt(self, arg):
        '''
         xpsSerdesHalt: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            runState = new_int32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesHalt(args[0],serdesId, runState)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('runState = %d for serdesId = %d' % (int32_tp_value(runState),serdesId))
                    pass
            delete_int32_tp(runState)

    #/********************************************************************************/
    # command for xpsSerdesPmdStatus
    #/********************************************************************************/
    def do_serdes_pmd_status(self, arg):
        '''
         xpsSerdesPmdStatus: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            status = new_int32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesPmdStatus(args[0],serdesId, status)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('status = %d for serdesId = %d' % (int32_tp_value(status),serdesId))
                    pass
    #/********************************************************************************/
    # command for xpsSerdesEyeGradientPlotWrite
    #/********************************************************************************/
    def do_serdes_eye_gradient_plot_write(self, arg):
        '''
         xpsSerdesEyeGradientPlotWrite: Enter [ devId,serdesId/startSerdesId-endSerdesId,fileName ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,fileName ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            fileName = args[2]
            # print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d, fileName=%s' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesEyeGradientPlotWrite(args[0],fileName,serdesId)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('fileName = %s is written for serdesId = %d' % (fileName,serdesId))
                    pass
    #/********************************************************************************/
    # command for xpsSerdesSetTxDatapathEncoding
    #/********************************************************************************/
    def do_serdes_set_tx_datapath_encoding(self, arg):
        '''
         xpsSerdesSetTxDatapathEncoding: Enter [ devId,grayCode,preCode,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,grayCode,preCode,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            if '-' in args[3]:
                startIdx = int(args[3].split('-')[0].strip())
                endIdx = int(args[3].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[3] = int(args[3])
                startIdx = args[3]
                endIdx = args[3]
            #print('Input Arguments are, devId=%d, grayCode=0x%x, preCode=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2],args[3]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSetTxDatapathEncoding(args[0],args[1],args[2],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesGetTxDatapathEncoding
    #/********************************************************************************/
    def do_serdes_get_tx_datapath_encoding(self, arg):
        '''
         xpsSerdesGetTxDatapathEncoding: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            grayCode_Ptr_1 = new_intp()
            preCode_Ptr_2 = new_intp()
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetTxDatapathEncoding(args[0],grayCode_Ptr_1,preCode_Ptr_2,serdesId)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('grayCode = %d for serdesId = %d' % (intp_value(grayCode_Ptr_1),serdesId))
                    print('preCode = %d for serdesId = %d' % (intp_value(preCode_Ptr_2),serdesId))
                    pass
            delete_intp(preCode_Ptr_2)
            delete_intp(grayCode_Ptr_1)
    #/********************************************************************************/
    # command for xpsSerdesSetRxDatapathEncoding
    #/********************************************************************************/
    def do_serdes_set_rx_datapath_encoding(self, arg):
        '''
         xpsSerdesSetRxDatapathEncoding: Enter [ devId,grayCode,preCode,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,grayCode,preCode,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            if '-' in args[3]:
                startIdx = int(args[3].split('-')[0].strip())
                endIdx = int(args[3].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[3] = int(args[3])
                startIdx = args[3]
                endIdx = args[3]
            #print('Input Arguments are, devId=%d, grayCode=0x%x, preCode=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2],args[3]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSetRxDatapathEncoding(args[0],args[1],args[2],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesGetRxDatapathEncoding
    #/********************************************************************************/
    def do_serdes_get_rx_datapath_encoding(self, arg):
        '''
         xpsSerdesGetRxDatapathEncoding: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            grayCode_Ptr_1 = new_intp()
            preCode_Ptr_2 = new_intp()
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],serdesId))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetRxDatapathEncoding(args[0],grayCode_Ptr_1,preCode_Ptr_2,serdesId)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('grayCode = %d for serdesId = %d' % (intp_value(grayCode_Ptr_1),serdesId))
                    print('preCode = %d for serdesId = %d' % (intp_value(preCode_Ptr_2),serdesId))
                    pass
            delete_intp(preCode_Ptr_2)
            delete_intp(grayCode_Ptr_1)

    #/********************************************************************************/
    # command for xpsSerdesGetRxData
    #/********************************************************************************/
    def do_serdes_get_rx_data(self, arg):
        '''
         xpsSerdesGetRxData: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            data = [0] * 4
            print('============================================================')
            print(' serdes-Id |  Data[0]  |  Data[1]  |  Data[2]  |  Data[3]  |')
            print('============================================================')
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetRxData(args[0], serdesId, data)
                if ret != 0:
                    print('Error : Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('    %03d    |  0x%05X  |  0x%05X  |  0x%05X  |  0x%05X  |' % (serdesId, data[0], data[1], data[2], data[3]))
            del data
            print('============================================================')

    #/********************************************************************************/
    # command for xpsSerdesGetMediaStatus
    #/********************************************************************************/
    def do_serdes_get_media_status(self, arg):
        '''
         xpsSerdesGetMediaStatus: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]

            isConnected = new_uint8_tp()
            print('============================')
            print(' serdes-Id |     status    |')
            print('============================')
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetMediaStatus(args[0], serdesId, isConnected)
                if ret == 0:
                    print '    %03d    | %13s |' %(serdesId , '\033[92m    Connected\033[00m' if \
                          (uint8_tp_value(isConnected) == 1) else '\033[91mNot Connected\033[00m')
                else:
                    print '    %03d    |     ERROR=%03d |' %(serdesId, ret)
            print('============================')
            delete_uint8_tp(isConnected)

    #/********************************************************************************/
    # command for xpsSerdesGetFrequencyLock
    #/********************************************************************************/
    def do_serdes_get_frequency_lock(self, arg):
        '''
         xpsSerdesGetFrequencyLock: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            frequencyLock = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetFrequencyLock(args[0],serdesId, frequencyLock)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('frequencyLock = %d for serdesId = %d' % (int32_tp_value(frequencyLock),serdesId))
                    pass
            delete_int32_tp(frequencyLock)
    #/********************************************************************************/
    # command for xpsSerdesGetSignalOkLive
    #/********************************************************************************/
    def do_serdes_get_signal_ok_live(self, arg):
        '''
         xpsSerdesGetSignalOkLive: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            isLive = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetSignalOkLive(args[0],serdesId,isLive)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('Signal status = %d for serdesId = %d' % (int32_tp_value(isLive),serdesId))
                    pass
            delete_int32_tp(isLive)
    #/********************************************************************************/
    # command for xpsSerdesGetErrorFlag
    #/********************************************************************************/
    def do_serdes_get_error_flag(self, arg):
        '''
         xpsSerdesGetErrorFlag: Enter [ devId,reset,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,reset,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            if '-' in args[2]:
                startIdx = int(args[2].split('-')[0].strip())
                endIdx = int(args[2].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[2] = int(args[2])
                startIdx = args[2]
                endIdx = args[2]
            #print('Input Arguments are, devId=%d, reset=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesGetErrorFlag(args[0],args[1],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesEnableCoreToControl
    #/********************************************************************************/
    def do_serdes_enable_core_to_control(self, arg):
        '''
         xpsSerdesEnableCoreToControl: Enter [ devId,txRxEnable,lowPowerEnable,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,txRxEnable,lowPowerEnable,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            if '-' in args[3]:
                startIdx = int(args[3].split('-')[0].strip())
                endIdx = int(args[3].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[3] = int(args[3])
                startIdx = args[3]
                endIdx = args[3]
            #print('Input Arguments are, devId=%d, txRxEnable=0x%x, lowPowerEnable=0x%x, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1],args[2],args[3]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesEnableCoreToControl(args[0],args[1],args[2],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesAddrInit
    #/********************************************************************************/
    def do_serdes_addr_init(self, arg):
        '''
         xpsSerdesAddrInit: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            addrStruct_Ptr_1 = new_xpSerdesAddr_tp(1)
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],serdesId))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesAddrInit(args[0],addrStruct_Ptr_1,serdesId)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('xpSerdesAddr.chip = %d for serdesId = %d' % (xpSerdesAddr_tp_getitem(addrStruct_Ptr_1, 0).chip,serdesId))
                    print('xpSerdesAddr.ring = %d for serdesId = %d' % (xpSerdesAddr_tp_getitem(addrStruct_Ptr_1, 0).ring,serdesId))
                    print('xpSerdesAddr.sbus = %d for serdesId = %d' % (xpSerdesAddr_tp_getitem(addrStruct_Ptr_1, 0).sbus,serdesId))
                    print('xpSerdesAddr.lane = %d for serdesId = %d' % (xpSerdesAddr_tp_getitem(addrStruct_Ptr_1, 0).lane,serdesId))
                    pass
            delete_xpSerdesAddr_tp(addrStruct_Ptr_1)
    #/********************************************************************************/
    # command for xpsSerdesSensorStartTemperature
    #/********************************************************************************/
    def do_serdes_sensor_start_temperature(self, arg):
        '''
         xpsSerdesSensorStartTemperature: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSensorStartTemperature(args[0],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesSensorWaitTemperature
    #/********************************************************************************/
    def do_serdes_sensor_wait_temperature(self, arg):
        '''
         xpsSerdesSensorWaitTemperature: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            temp = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSensorWaitTemperature(args[0],serdesId, temp)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('Temperature = %d for serdesId = %d' % (int32_tp_value(temp),serdesId))
                    pass
            delete_int32_tp(temp)
    #/********************************************************************************/
    # command for xpsSerdesSensorGetVoltage
    #/********************************************************************************/
    def do_serdes_sensor_get_voltage(self, arg):
        '''
         xpsSerdesSensorGetVoltage: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            mVolt = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSensorGetVoltage(args[0],serdesId, mVolt)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('mVolt = %d for serdesIf = %d' % (int32_tp_value(mVolt),serdesId))
                    pass
            delete_int32_tp(mVolt)
    #/********************************************************************************/
    # command for xpsSerdesSensorStartVoltage
    #/********************************************************************************/
    def do_serdes_sensor_start_voltage(self, arg):
        '''
         xpsSerdesSensorStartVoltage: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSensorStartVoltage(args[0],serdesId)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
    #/********************************************************************************/
    # command for xpsSerdesSensorWaitVoltage
    #/********************************************************************************/
    def do_serdes_sensor_wait_voltage(self, arg):
        '''
         xpsSerdesSensorWaitVoltage: Enter [ devId,serdesId/startSerdesId-endSerdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            mVolt = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d' % (args[0],args[1]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesSensorWaitVoltage(args[0],serdesId,mVolt)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    print('mVolt = %d for serdesId = %d' % (int32_tp_value(mVolt),serdesId))
                    pass
            delete_int32_tp(mVolt)
    #/********************************************************************************/
    # command for xpsSerdesBroadcastIntWMask
    #/********************************************************************************/
    def do_serdes_broadcast_int_w_mask(self, arg):
        '''
         xpsSerdesBroadcastIntWMask: Enter [ devId,serdesId/startSerdesId-endSerdesId,addrMask,intNum,param,args ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,addrMask,intNum,param,args ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            varArgs = new_intArr(len(args)-5)
            for index in range(len(args)-5):
                intArr_setitem(varArgs, index, int(args[5+index]))
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d, addrMask=0x%x, intNum=0x%x, param=0x%x, args=0x%x' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesBroadcastIntWMask(args[0],serdesId,args[2],args[3],args[4],len(args)-5, varArgs)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
            delete_intArr(varArgs)
    #/********************************************************************************/
    # command for xpsSerdesBroadcastInt
    #/********************************************************************************/
    def do_serdes_broadcast_int(self, arg):
        '''
         xpsSerdesBroadcastInt: Enter [ devId,serdesId/startSerdesId-endSerdesId,intNum,param,args ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,intNum,param,args ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            args[2] = int(args[2])
            args[3] = int(args[3])
            varArgs = new_intArr(len(args)-4)
            for index in range(len(args)-4):
                intArr_setitem(varArgs, index, int(args[4+index]))
            #print('Input Arguments are, devId=%d, serdesId/startSerdesId-endSerdesId=%d, intNum=0x%x, param=0x%x, args=0x%x' % (args[0],args[1],args[2],args[3],args[4]))
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesBroadcastInt(args[0],serdesId,args[2],args[3],len(args)-4,varArgs)
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                else:
                    pass
            delete_intArr(varArgs)

    #/********************************************************************************/
    # command for xpsSerdesElectricalIdleThresholdSet
    #/********************************************************************************/
    def do_serdes_electrical_idle_threshold_set(self, arg):
        '''
         xpsSerdesElectricalIdleThresholdSet: Enter [ devId,serdesId/startSerdesId-endSerdesId,eiThreshold]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId,eiThreshold ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            args[2] = int(args[2])
            intstatus_Ptr_3 = new_uint32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesElectricalIdleThresholdSet(args[0],serdesId,args[2],intstatus_Ptr_3)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('intstatus = %d for serdesId = %d' % (uint32_tp_value(intstatus_Ptr_3),serdesId))
                    pass
            delete_uint32_tp(intstatus_Ptr_3)


    #/********************************************************************************/
    # command for xpsSerdesPmroMetricGet
    #/********************************************************************************/
    def do_serdes_pmro_metric_get(self, arg):
        '''
         xpsSerdesPmroMetricGet: Enter [ devId,serdesId/startSerdesId-endSerdesId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            pmroMetric_ptr = new_uint32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesPmroMetricGet(args[0],serdesId,pmroMetric_ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('pmroMetric = %d for serdesId = %d' % (uint32_tp_value(pmroMetric_ptr),serdesId))
                    pass
            delete_uint32_tp(pmroMetric_ptr)

    #/********************************************************************************/
    # command for xpsSerdesDfeStatusGet
    #/********************************************************************************/
    def do_serdes_dfe_status_get(self, arg):
        '''
         xpsSerdesDfeStatusGet: Enter [ devId,serdesId/startSerdesId-endSerdesId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            dfeStatus_ptr = new_int32_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesDfeStatusGet(args[0],serdesId,dfeStatus_ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    print('dfeStatus = %d for serdesId = %d' % (int32_tp_value(dfeStatus_ptr),serdesId))
                    pass
            delete_int32_tp(dfeStatus_ptr)

    #/********************************************************************************/
    # command for xpsSerdesDfeStateGet
    #/********************************************************************************/
    def do_serdes_dfe_state_get(self, arg):
        '''
         xpsSerdesDfeStateGet: Enter [ devId,serdesId/startSerdesId-endSerdesId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId/startSerdesId-endSerdesId ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            dfeState_ptr = new_xpSerdesDfeState_tp()
            for serdesId in range(startIdx,endIdx+1):
                ret = xpsSerdesDfeStateGet(args[0],serdesId,dfeState_ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                    err = 1
                if err == 0:
                    ret = xpsSerdesDfeStatePrint(args[0],serdesId,dfeState_ptr,0)
                    err = 0
                    if ret != 0:
                        print('Return Value = %d for serdesId = %d' % (ret,serdesId))
                        err = 1
                    if err == 0:
                        pass
            delete_xpSerdesDfeState_tp(dfeState_ptr)

    #/********************************************************************************/
    # command for xpsSerdesEyeDataReadPhase
    #/********************************************************************************/
    def do_serdes_eye_data_read_phase(self, arg):
        '''
         xpsSerdesEyeDataReadPhase: Enter [ devId,serdesId,fileName ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId,fileName ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = str(args[2])
            readPhaseVal_Ptr_3 = new_int32_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d, Filename=%s' % (args[0],args[1],args[2]))
            ret = xpsSerdesEyeDataReadPhase(args[0],args[1],args[2],readPhaseVal_Ptr_3)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('Serdes eye data read phase value = %d' % (int32_tp_value(readPhaseVal_Ptr_3)))

    #/********************************************************************************/
    # command for xpsSerdesDfeLos
    #/********************************************************************************/
    def do_serdes_dfe_los(self, arg):
        '''
         xpsSerdesDfeLos: Enter [ devId,serdesId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serdesId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            dfeLosVal_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, serdesId=%d' % (args[0],args[1]))
            ret = xpsSerdesDfeLos(args[0],args[1],dfeLosVal_Ptr_2)
            if ret == 0:
                if ((uint8_tp_value(dfeLosVal_Ptr_2)) == 0):
                    print('dfe LOS is not detected')
                else:
                    print('dfe LOS is detected')
            else:
                print('Return Value = %d' % (ret))


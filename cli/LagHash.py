# LagHash.py
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
import re

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")

from cmd2x import Cmd
import enumDict

#/*********************************************************************************************************/
# import everything from buildTarget
from buildTarget import *

#/*********************************************************************************************************/
# The class object for lag hash fields  commands
#/*********************************************************************************************************/
class runHashFieldCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # Auto completion for hasField Enums
    #/********************************************************************************/
    def complete_xp_set_lag_hash_fields(self, text, line, begidx, endidx):
        tempDict = { 2 : 'hashField'}
        return enumDict.autoCompletion(line , text, tempDict)

    #/*****************************************************************************************************/
    # set Hash field in lag
    #/*****************************************************************************************************/
    def do_xp_set_lag_hash_fields(self, arg):
        '''
         xpSetLagHashFields: Enter [ deviceId, hashList ]
         Valid values for hashField : <hashField>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,hashList ]')
        else:
            args[0] = int(args[0])
            postList = args[1].strip("]").strip("[").split(",")
            listLen = len(postList)
            for hashFieldEnum in postList:
                hashFieldEnum = hashFieldEnum.strip()
                if hashFieldEnum not in enumDict.enumDictionaries['hashField'].keys():
                    print "invalid data %s"%(hashFieldEnum)
                    return
            print('Input Arguments are, deviceId=%d hashList %s' % (args[0], args[1]))
            fields_Ptr_1 = new_xpHashField_arr(listLen)
            for i in range(0, listLen):
                xpHashField_arr_setitem(fields_Ptr_1, i, eval(postList[i]))
            ret = xpLagSetHashFields(args[0],fields_Ptr_1,listLen)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')
            delete_xpHashField_arr(fields_Ptr_1)
    def help_xp_set_lag_hash_fields(self):
        print '\n'.join( ['xp_set_lag_hash_fields: Enter [deviceId, hashList]',
                'example=> xp_set_lag_hash_fields 0 [ETHERNET_C_TAG,GENEVE_VNI]',
                'example=> xp_set_lag_hash_fields 0 ETHERNET_C_TAG,GENEVE_VNI'
                ])

    #/********************************************************************************/
    # command for xpSetLagHashPolynomial
    #/********************************************************************************/
    def do_xp_set_lag_hash_polynomial(self, arg):
        '''
         xpSetLagHashPolynomial: Enter [ deviceId,instance,polynomialId ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d' % (args[0],args[1],args[2]))
            ret = xpSetLagHashPolynomial(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')

    #/********************************************************************************/
    # command for xpGetLagHashPolynomial
    #/********************************************************************************/
    def do_xp_get_lag_hash_polynomial(self, arg):
        '''
         xpGetLagHashPolynomial: Enter [ deviceId,instance ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpGetLagHashPolynomial(args[0],args[1],polynomialId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                print('Command Success')
            delete_uint32_tp(polynomialId_Ptr_2)

    #/********************************************************************************/
    # command for xpSetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_xp_set_lag_hash_polynomial_seed(self, arg):
        '''
         xpSetLagHashPolynomialSeed: Enter [ deviceId,instance,seed ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            print('Input Arguments are, deviceId=%d, instance=%d, seed=%d' % (args[0],args[1],args[2]))
            ret = xpSetLagHashPolynomialSeed(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Command Success')

    #/********************************************************************************/
    # command for xpGetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_xp_get_lag_hash_polynomial_seed(self, arg):
        '''
         xpGetLagHashPolynomialSeed: Enter [ deviceId,instance ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            seed_Ptr_2 = new_uint32_tp()
            print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpGetLagHashPolynomialSeed(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint32_tp_value(seed_Ptr_2)))
                print('Command Success')
            delete_uint32_tp(seed_Ptr_2)



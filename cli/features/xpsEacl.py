#!/usr/bin/env python
#  xpsEacl.py
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
# The class object for xpsEacl operations
#/**********************************************************************************/

class xpsEaclObj(Cmd):

    mapXpEaclV4KeyByteMask = {
        0 : 0x1,   #XP_EACL_KEY_TYPE_V4
        1 : 0x3f,  #XP_EACL_MAC_DA
        2 : 0x3f,  #XP_EACL_MAC_SA
        3 : 0xf,   #XP_EACL_DIP_V4
        4 : 0xf,   #XP_EACL_SIP_V4
        5 : 0x3,   #XP_EACL_L4_DEST_PORT
        6 : 0x3,   #XP_EACL_L4_SRC_PORT
        7 : 0x3,   #XP_EACL_V4_ETHER_TYPE
        8 : 0x1,   #XP_EACL_ICMP_MSG_TYPE
        9 : 0x1,   #XP_EACL_PROTOCOL
        10 : 0x03, #XP_EACL_EGRESS_BD
        11 : 0x3,  #XP_EACL_EGRESS_VIF
        12 : 0x7,  #XP_EACL_VI
        13 : 0x3,  #XP_EACL_DSCP_PCP_EXP
        14 : 0x7,  #XP_EACL_TCP_FLAGS_RSNCODE
    }
    mapXpEaclV6KeyByteMask = {
        0 : 0x1,     #'XP_EACL_KEY_TYPE_V6'     
        1 : 0xffff,  #'XP_EACL_DIP_V6'          
        2 : 0xffff,  #'XP_EACL_SIP_V6'          
        3 : 0x3,     #'XP_EACL_L4_V6_DEST_PORT' 
        4 : 0x3,     #'XP_EACL_L4_V6_SRC_PORT'  
        5 : 0x3,     #'XP_EACL_V6_ETHER_TYPE'   
        6 : 0x1,     #'XP_EACL_ICMP_V6_MSG_TYPE'
        7 : 0x3,     #'XP_EACL_V6_EGRESS_BD'    
        8 : 0x3,     #'XP_EACL_V6_EGRESS_VIF'   
        9 : 0x3,     #'XP_EACL_V6_RSNCODE_DSCP' 
    }

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsEaclInit
    #/********************************************************************************/

    def do_eacl_init(self, arg):
        '''
         xpsEaclInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsEaclInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclDeInit
    #/********************************************************************************/
    def do_eacl_de_init(self, arg):
        '''
         xpsEaclDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsEaclDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsEaclInitScope
    #/********************************************************************************/
    def do_eacl_init_scope(self, arg):
        '''
         xpsEaclInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsEaclInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclDeInitScope
    #/********************************************************************************/
    def do_eacl_de_init_scope(self, arg):
        '''
         xpsEaclDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsEaclDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclAddDevice
    #/********************************************************************************/
    def do_eacl_add_device(self, arg):
        '''
         xpsEaclAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsEaclAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclRemoveDevice
    #/********************************************************************************/
    def do_eacl_remove_device(self, arg):
        '''
         xpsEaclRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclDefineKey
    #/********************************************************************************/
    def do_eacl_define_key(self, arg):
        '''
         xpsEaclDefineKey: Enter [ devId,isValid,numFlds,type,flds]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isValid,numFlds,type,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. For example check eacl_help command')
        else:
            args[0] = int(args[0])
            fldList = args[4].strip().split(',')
            xpEaclkeyFieldList_t_Ptr = new_xpEaclkeyFieldList_tp()
            xpEaclkeyField_t_ptr = new_xpEaclkeyField_tp()
            xpEaclkeyField_t_arr = new_xpEaclkeyField_arr(len(fldList))
            xpEaclkeyFieldList_t_Ptr.isValid = int(args[1])
            xpEaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpEaclkeyFieldList_t_Ptr.type = eval(args[3])
            if len(fldList) == int(args[2]):
                for fldIdx,fldEnm in enumerate(fldList):
                       if(xpEaclkeyFieldList_t_Ptr.type == 0):
                           xpEaclkeyField_t_ptr.fld.v4Fld = eval(fldEnm)
                           byteSize = self.mapXpEaclV4KeyByteMask[eval(fldEnm)]
                       else:
                           xpEaclkeyField_t_ptr.fld.v6Fld = eval(fldEnm)
                           byteSize = self.mapXpEaclV6KeyByteMask[eval(fldEnm)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       xpEaclkeyField_t_ptr.value = valPtr 
                       xpEaclkeyField_t_ptr.mask = maskPtr 
                       xpEaclkeyField_arr_setitem(xpEaclkeyField_t_arr, fldIdx, xpEaclkeyField_t_ptr) 
                   
                xpEaclkeyFieldList_t_Ptr.fldList = xpEaclkeyField_t_arr
                #print('Input Arguments are, devId=%d, isValid=%d, numFlds=%d, type=%d fld=%s value=%s mask=%s' % (args[0],xpEaclkeyFieldList_t_Ptr.isValid,xpEaclkeyFieldList_t_Ptr.numFlds,xpEaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                ret = xpsEaclDefineKey(args[0],xpEaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                del valPtr
                del maskPtr
                del xpEaclkeyField_t_arr,
                del xpEaclkeyField_t_ptr
                delete_xpEaclkeyFieldList_tp(xpEaclkeyFieldList_t_Ptr)
            else:
                print "Insufficient arguments"
                pass

    #/********************************************************************************/
    # command for xpsEaclWriteKey
    #/********************************************************************************/
    def do_eacl_write_key(self, arg):
        '''
         xpsEaclWriteKey: Enter [ devId,camIndex,isValid,numFlds,type, flds,value,mask flds,value,mask ... upto the num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,isValid,numFlds,type,flds,value,mask flds,value,mask... append flds,value,mask for num of key wants to be program.] \n \
			e.g.:- \n \
			eacl_write_key 0 1 1 4 XP_EACL_V4_TYPE XP_EACL_KEY_TYPE_V4,0,0 XP_EACL_PROTOCOL,6,0 XP_EACL_EGRESS_VIF,38,0 XP_EACL_DSCP_PCP_EXP,11,0')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[3]) and len(valueList) == int(args[3]) and len(maskList) == int(args[3]):
                xpEaclkeyFieldList_t_Ptr = new_xpEaclkeyFieldList_tp()
                xpEaclkeyField_t_ptr = new_xpEaclkeyField_tp()
                xpEaclkeyField_t_arr = new_xpEaclkeyField_arr(len(fldList))
                xpEaclkeyFieldList_t_Ptr.isValid = int(args[2])
                xpEaclkeyFieldList_t_Ptr.numFlds = len(fldList)
                xpEaclkeyFieldList_t_Ptr.type = eval(args[4])

                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpEaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                           byteSize = self.mapXpEaclV4KeyByteMask[eval(fldList[fldIdx])]
                       else:
                           xpEaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                           byteSize = self.mapXpEaclV6KeyByteMask[eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       memZero(valPtr, sizeof_uint8 * byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       memZero(maskPtr, sizeof_uint8 * byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split('.')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['255'] * len(fldVal)
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split(':')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if (not(maskList[fldIdx] == '1' or maskList[fldIdx] == '0')):
                                   tmp1 = []
                                   tmp =  bin(int(maskList[fldIdx])).lstrip('0b')
                                   if (len(tmp) <= 8 ):
                                      fldMask.append(hex(int(tmp[:8],2)).replace('0x',''))
                                   else:
                                      for idx in range(byteLen):
                                          fldMask.append(hex((int(maskList[fldIdx]) >> (8*idx)) & 0xff).replace('0x',''))
                                      fldMask.reverse()
                               elif maskList[fldIdx] == '0':
                                   fldMask = ['00'] * len(fldVal)
                               elif maskList[fldIdx] == '1':
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           #valPtr = new_uint8Arr_tp(byteLen)
                           #maskPtr = new_uint8Arr_tp(byteLen)
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                           #print 'fldMask:',fldMask
                           #print 'fldVal:',fldVal
                       xpEaclkeyField_t_ptr.value = valPtr 
                       xpEaclkeyField_t_ptr.mask = maskPtr 
                       xpEaclkeyField_arr_setitem(xpEaclkeyField_t_arr, fldIdx, xpEaclkeyField_t_ptr) 
                xpEaclkeyFieldList_t_Ptr.fldList = xpEaclkeyField_t_arr
                #print('Input Arguments are, devId=%d \ncamIndex=%d \nisValid=%d \nnumFlds=%d \ntype=%d \nfld=%s \nvalue=%s \n mask=%s' % (args[0],args[1],xpEaclkeyFieldList_t_Ptr.isValid,xpEaclkeyFieldList_t_Ptr.numFlds,xpEaclkeyFieldList_t_Ptr.type, fldList, valueList, maskList))
                ret = xpsEaclWriteKey(args[0],args[1],xpEaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpEaclkeyFieldList_tp(xpEaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsEaclCreateTable
    #/********************************************************************************/
    def do_eacl_create_table(self, arg):
        '''
         xpsEaclCreateTable: Enter [ devId,numTables,numDb,keySize]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numTables,numDb,keySize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            defKeySize = int(args[3])
            if(defKeySize <= 64):
                keySize = 65 
            elif(defKeySize > 64 and defKeySize <= 128):
                keySize = 130 
            elif(defKeySize > 128 and defKeySize <= 192):
                keySize = 195
            else:
                keySize = 390
            xpEaclTableProfile_t_Ptr = xpEaclTableProfile_t()
            xpEaclTableProfile_t_Ptr.numTables = int(args[1])
            xpEaclTableProfile_t_Ptr.tableProfile.keySize = keySize 
            xpEaclTableProfile_t_Ptr.tableProfile.numDb = int(args[2])
            #print('Input Arguments are, devId=%d, numTables=%d, keySize=%d, numDb=%d' % (args[0],xpEaclTableProfile_t_Ptr.numTables,xpEaclTableProfile_t_Ptr.tableProfile.keySize,xpEaclTableProfile_t_Ptr.tableProfile.numDb))
            ret = xpsEaclCreateTable(args[0],xpEaclTableProfile_t_Ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclDeleteTable
    #/********************************************************************************/
    def do_eacl_delete_table(self, arg):
        '''
         xpsEaclDeleteTable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclDeleteTable(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                xpsEaclObj.DefineFldList = []
                pass
        

    #/********************************************************************************/
    # command for xpsEaclWriteData
    #/********************************************************************************/
    def do_eacl_write_data(self, arg):
        '''
         xpsEaclWriteData: Enter [ devId,camIndex,enPktCmdUpd,enRsnCodeUpd,pktCmd,rsnCode,remarkDscp,remarkExp,remarkPcp,mirrorSsnUpd,mirrorSessionId,enCnt,enPolicer,dscpVal,espVal,pcpVal,counterId,policerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 17
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,enPktCmdUpd,enRsnCodeUpd,pktCmd,rsnCode,remarkDscp,remarkExp,remarkPcp,mirrorSsnUpd,mirrorSessionId,enCnt,enPolicer,dscpVal,espVal,pcpVal,counterId,policerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsEaclData_t_Ptr = new_xpsEaclData_tp()
            xpsEaclData_t_Ptr.enPktCmdUpd = int(args[2])
            xpsEaclData_t_Ptr.enRsnCodeUpd = int(args[3])
            xpsEaclData_t_Ptr.pktCmd = int(args[4])
            xpsEaclData_t_Ptr.rsnCode = int(args[5])
            xpsEaclData_t_Ptr.remarkDscp = int(args[6])
            xpsEaclData_t_Ptr.remarkExp = int(args[7])
            xpsEaclData_t_Ptr.remarkPcp = int(args[8])
            xpsEaclData_t_Ptr.mirrorSsnUpd = int(args[9])
            xpsEaclData_t_Ptr.mirrorSessionId = int(args[10])
            xpsEaclData_t_Ptr.enCnt = int(args[11])
            xpsEaclData_t_Ptr.enPolicer = int(args[12])
            xpsEaclData_t_Ptr.dscpVal = int(args[13])
            xpsEaclData_t_Ptr.espVal = int(args[14])
            xpsEaclData_t_Ptr.pcpVal = int(args[15])
            xpsEaclData_t_Ptr.counterId = int(args[16])
            xpsEaclData_t_Ptr.policerId = int(args[17])
            #print('Input Arguments are, devId=%d, camIndex=%d, enPktCmdUpd=%d, enRsnCodeUpd=%d, pktCmd=%d, rsnCode=%d, remarkDscp=%d, remarkExp=%d, remarkPcp=%d, mirrorSsnUpd=%d, mirrorSessionId=%d, enCnt=%d, enPolicer=%d, dscpVal=%d, espVal=%d, pcpVal=%d, counterId=%d, policerId=%d' % (args[0],args[1],xpsEaclData_t_Ptr.enPktCmdUpd,xpsEaclData_t_Ptr.enRsnCodeUpd,xpsEaclData_t_Ptr.pktCmd,xpsEaclData_t_Ptr.rsnCode,xpsEaclData_t_Ptr.remarkDscp,xpsEaclData_t_Ptr.remarkExp,xpsEaclData_t_Ptr.remarkPcp,xpsEaclData_t_Ptr.mirrorSsnUpd,xpsEaclData_t_Ptr.mirrorSessionId,xpsEaclData_t_Ptr.enCnt,xpsEaclData_t_Ptr.enPolicer,xpsEaclData_t_Ptr.dscpVal,xpsEaclData_t_Ptr.espVal,xpsEaclData_t_Ptr.pcpVal,xpsEaclData_t_Ptr.counterId,xpsEaclData_t_Ptr.policerId))
            ret = xpsEaclWriteData(args[0],args[1],xpsEaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsEaclData_tp(xpsEaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsEaclWriteEntry
    #/********************************************************************************/
    def do_eacl_write_entry(self, arg):
        '''
         xpsEaclWriteEntry: Enter [ devId,camIndex,isValid,numFlds,type,enPktCmdUpd,enRsnCodeUpd,pktCmd,rsnCode,remarkDscp,remarkExp,remarkPcp,mirrorSsnUpd,mirrorSessionId,enCnt,enPolicer,dscpVal,espVal,pcpVal,counterId,policerId flds,value,mask flds,value,mask ... upto the num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 22
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,isValid,numFlds,type,enPktCmdUpd,enRsnCodeUpd,pktCmd,rsnCode,remarkDscp,remarkExp,remarkPcp,mirrorSsnUpd,mirrorSessionId,enCnt,enPolicer,dscpVal,espVal,pcpVal,counterId,policerId flds,value,mask flds,value,mask ... append flds,value,mask for num of key wants to be program.] \n \
			e.g.:- \n \
			eacl_write_entry 0 1 1 4 XP_EACL_V4_TYPE 1 1 0 202 0 0 0 1 1 0 0 0 0 0 0 0 XP_EACL_KEY_TYPE_V4,0,0 XP_EACL_PROTOCOL,6,0 XP_EACL_EGRESS_VIF,38,0 XP_EACL_DSCP_PCP_EXP,11,0')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[3]) and len(valueList) == int(args[3]) and len(maskList) == int(args[3]):
                xpEaclkeyFieldList_t_Ptr = new_xpEaclkeyFieldList_tp()
                xpEaclkeyField_t_ptr = new_xpEaclkeyField_tp()
                xpEaclkeyField_t_arr = new_xpEaclkeyField_arr(len(fldList))
                xpEaclkeyFieldList_t_Ptr.isValid = int(args[2])
                xpEaclkeyFieldList_t_Ptr.numFlds = int(args[3])
                xpEaclkeyFieldList_t_Ptr.type = eval(args[4])
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpEaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                           byteSize = self.mapXpEaclV4KeyByteMask[eval(fldList[fldIdx])]
                       else:
                           xpEaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                           byteSize = self.mapXpEaclV6KeyByteMask[eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split('.')
                               if(maskList[fldIdx] == 0):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['255'] * len(fldVal)
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split(':')
                               if(maskList[fldIdx] == 0):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if(fldVal[0] == '00' ):
                                   fldMask = ['00']
                               if (not(maskList[fldIdx] == '1' or maskList[fldIdx] == '0')):
                                   fldMask.append(hex(int(maskList[fldIdx])).replace('0x',''))
                               else:
                                   tmpMask = str('1' * ((byteLen*8)- len(bin(int(valueList[fldIdx])).lstrip('0b'))) + str(maskList[fldIdx] * len(bin(int(valueList[fldIdx])).lstrip('0b'))))
                                   tmpMask = hex(int(tmpMask, 2))
                                   tmpMask = format(int(tmpMask,16), '0' + str(byteLen*2) + 'x')
                                   fldMask = [tmpMask[i:i+2] for i in range(0, len(tmpMask), 2)] 
                               hexFlag = 1
                           #valPtr = new_uint8Arr_tp(byteLen)
                           #maskPtr = new_uint8Arr_tp(byteLen)
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpEaclkeyField_t_ptr.value = valPtr 
                       xpEaclkeyField_t_ptr.mask = maskPtr 
                       xpEaclkeyField_arr_setitem(xpEaclkeyField_t_arr, fldIdx, xpEaclkeyField_t_ptr) 
                xpEaclkeyFieldList_t_Ptr.fldList = xpEaclkeyField_t_arr

                #print('Input Arguments are, devId=%d \nisValid=%d \nnumFlds=%d \ntype=%d \nfld=%s \nvalue=%s \nmask=%s' % (args[0],xpEaclkeyFieldList_t_Ptr.isValid,xpEaclkeyFieldList_t_Ptr.numFlds,xpEaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                xpsEaclData_t_Ptr = new_xpsEaclData_tp()
                xpsEaclData_t_Ptr.enPktCmdUpd = int(args[5])
                xpsEaclData_t_Ptr.enRsnCodeUpd = int(args[6])
                xpsEaclData_t_Ptr.pktCmd = int(args[7])
                xpsEaclData_t_Ptr.rsnCode = int(args[8])
                xpsEaclData_t_Ptr.remarkDscp = int(args[9])
                xpsEaclData_t_Ptr.remarkExp = int(args[10])
                xpsEaclData_t_Ptr.remarkPcp = int(args[11])
                xpsEaclData_t_Ptr.mirrorSsnUpd = int(args[12])
                xpsEaclData_t_Ptr.mirrorSessionId = int(args[13])
                xpsEaclData_t_Ptr.enCnt = int(args[14])
                xpsEaclData_t_Ptr.enPolicer = int(args[15])
                xpsEaclData_t_Ptr.dscpVal = int(args[16])
                xpsEaclData_t_Ptr.espVal = int(args[17])
                xpsEaclData_t_Ptr.pcpVal = int(args[18])
                xpsEaclData_t_Ptr.counterId = int(args[19])
                xpsEaclData_t_Ptr.policerId = int(args[20])
                #print('Input Arguments are, devId=%d, \ncamIndex=%d, \nisValid=%d, \nnumFlds=%d, \ntype=%d, \nfld=%s \nvalue=%s \nmask=%s \nenPktCmdUpd=%d \nenRsnCodeUpd=%d \npktCmd=%d \nrsnCode=%d \nremarkDscp=%d \nremarkExp=%d \nremarkPcp=%d \nmirrorSsnUpd=%d \nmirrorSessionId=%d \nenCnt=%d \nenPolicer=%d \ndscpVal=%d \nespVal=%d \npcpVal=%d \ncounterId=%d \npolicerId=%d' % (args[0],args[1],xpEaclkeyFieldList_t_Ptr.isValid,xpEaclkeyFieldList_t_Ptr.numFlds,xpEaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList,xpsEaclData_t_Ptr.enPktCmdUpd,xpsEaclData_t_Ptr.enRsnCodeUpd,xpsEaclData_t_Ptr.pktCmd,xpsEaclData_t_Ptr.rsnCode,xpsEaclData_t_Ptr.remarkDscp,xpsEaclData_t_Ptr.remarkExp,xpsEaclData_t_Ptr.remarkPcp,xpsEaclData_t_Ptr.mirrorSsnUpd,xpsEaclData_t_Ptr.mirrorSessionId,xpsEaclData_t_Ptr.enCnt,xpsEaclData_t_Ptr.enPolicer,xpsEaclData_t_Ptr.dscpVal,xpsEaclData_t_Ptr.espVal,xpsEaclData_t_Ptr.pcpVal,xpsEaclData_t_Ptr.counterId,xpsEaclData_t_Ptr.policerId))
                ret = xpsEaclWriteEntry(args[0],args[1],xpEaclkeyFieldList_t_Ptr,xpsEaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsEaclData_tp(xpsEaclData_t_Ptr)
                delete_xpEaclkeyFieldList_tp(xpEaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsEaclUpdateKeyField
    #/********************************************************************************/
    def do_eacl_update_key_field(self, arg):
        '''
         xpsEaclUpdateKeyField: Enter [ devId,camIndex,fldType,fldVal,fldMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,fldType,fldVal,fldMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpEaclKeyFieldId_t_ptr = xpEaclKeyFieldId_t()
            v4FldList = ['XP_EACL_KEY_TYPE_V4','XP_EACL_MAC_DA','XP_EACL_MAC_SA','XP_EACL_DIP_V4','XP_EACL_SIP_V4','XP_EACL_L4_DEST_PORT','XP_EACL_L4_SRC_PORT','XP_EACL_V4_ETHER_TYPE','XP_EACL_ICMP_MSG_TYPE','XP_EACL_PROTOCOL','XP_EACL_EGRESS_BD','XP_EACL_EGRESS_VIF','XP_EACL_VI','XP_EACL_DSCP_PCP_EXP','XP_EACL_TCP_FLAGS_RSNCODE']
            v6FldList = ['XP_EACL_KEY_TYPE_V6','XP_EACL_DIP_V6','XP_EACL_SIP_V6','XP_EACL_L4_V6_DEST_PORT','XP_EACL_L4_V6_SRC_PORT','XP_EACL_V6_ETHER_TYPE','XP_EACL_ICMP_V6_MSG_TYPE','XP_EACL_V6_PROTOCOL','XP_EACL_V6_EGRESS_BD','XP_EACL_V6_EGRESS_VIF','XP_EACL_V6_RSNCODE_DSCP','XP_EACL_V6_PCP_EXP']
            if(args[2] in v4FldList):
                xpEaclKeyFieldId_t_ptr.v4Fld = eval(args[2])
                byteSize =self.mapXpEaclV4KeyByteMask[eval(args[2])]
            elif(args[2] in v6FldList):
                xpEaclKeyFieldId_t_ptr.v6Fld = eval(args[2])
                byteSize = self.mapXpEaclV6KeyByteMask[eval(args[2])]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)

            fldVal = []
            hexFlg = 0
            if('.' in args[3]):
                fldVal = args[3].strip().split('.')
                if(args[4] == 0):
                    fldMask = ['00'] * len(fldVal)
                else:
                    fldMask = ['255'] * len(fldVal)
            elif(':' in args[3]):
                hexFlg = 1
                fldVal = args[3].strip().split(':')
                if(args[4] == 0):
                    fldMask = ['00'] * len(fldVal)
                else:
                    fldMask = ['ff'] * len(fldVal)
            else:
                tmpValue = format(int(args[3]), '0' +str(byteLen*2) + 'X')
                fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                tmpMask = str('1' * ((byteLen*8)- len(bin(int(args[3])).lstrip('0b'))) + str(args[4] * len(bin(int(args[3])).lstrip('0b'))))
                tmpMask = hex(int(tmpMask, 2))
                tmpMask = format(int(tmpMask,16), '0' + str(byteLen*2) + 'x')
                fldMask = [tmpMask[i:i+2] for i in range(0, len(tmpMask), 2)] 
                hexFlg = 1
            #args[2] = int(args[2])
            for valIdx in range(byteLen):
                if(hexFlg): 
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1],16))
                else:
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1]))
            hexFlg = 0
            #print('Input Arguments are, devId=%d, camIndex=%d, fldType=%s' % (args[0],args[1],eval(args[2])))
            ret = xpsEaclUpdateKeyField(args[0],args[1],xpEaclKeyFieldId_t_ptr,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('fldType = %d' % eval(args[2]))
                print('fieldVal = %s' % (args[3]))
                print('fieldMask = %d' % int(args[4]))
                print('Command Success')
                pass
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)


    #/********************************************************************************/
    # command for xpsEaclFreeKeyFieldList
    #/********************************************************************************/
    def do_eacl_free_key_field_list(self, arg):
        '''
         xpsEaclFreeKeyFieldList: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsEaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsEaclkeyFieldList_arr(1)
            xpsEaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0, fieldList_Ptr_2)

            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsEaclFreeKeyFieldList(args[0],args[1],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsEaclkeyFieldList_tp(fieldList_Ptr_2)

    #/********************************************************************************/
    # command for xpsEaclMallocKeyFieldList
    #/********************************************************************************/
    def do_eacl_malloc_key_field_list(self, arg):
        '''
         xpsEaclMallocKeyFieldList: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsEaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsEaclkeyFieldList_arr(1)
            xpsEaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            ret = xpsEaclPaclMallocKeyFieldList(args[0],args[1],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsEaclkeyFieldList_tp(fieldList_Ptr_2)

    #/********************************************************************************/
    # command for xpsEaclGetKeySize
    #/********************************************************************************/
    def do_eacl_get_key_size(self, arg):
        '''
         xpsEaclGetKeySize: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            keySize_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclGetKeySize(args[0],keySize_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keySize = %d' % (uint32_tp_value(keySize_Ptr_1)))
                pass
            delete_uint32_tp(keySize_Ptr_1)

    #/********************************************************************************/
    # command for xpsEaclGetKeyNumFlds
    #/********************************************************************************/
    def do_eacl_get_key_num_flds(self, arg):
        '''
         xpsEaclGetKeyNumFlds: Enter [ devId,keyType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numFlds_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, keyType=%d' % (args[0],args[1]))
            ret = xpsEaclGetKeyNumFlds(args[0],args[1],numFlds_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (uint32_tp_value(numFlds_Ptr_2)))
                pass

    #/********************************************************************************/
    # command for xpsEaclGetEntryKeyMask
    #/********************************************************************************/
    def do_eacl_get_entry_key_mask(self, arg):
        '''
         xpsEaclGetEntryKeyMask: Enter [ devId,camIndex,fld ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,fld ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpEaclKeyMask_t_Ptr = new_xpEaclKeyMask_tp()
            keySize = new_uint32_tp()
            ret = xpsEaclGetKeySize(args[0],keySize )
            byteLen = uint32_tp_value(keySize) / 8
            xpEaclKeyMask_t_Ptr.value = new_uint8Arr_tp(byteLen)
            xpEaclKeyMask_t_Ptr.mask = new_uint8Arr_tp(byteLen)
            fldList = args[2].strip().split(',')
            #print('Input Arguments are, devId=%d, camIndex=%d fldList=%s' % (args[0],args[1],fldList))
            ret = xpsEaclGetEntryKeyMask(args[0],args[1],xpEaclKeyMask_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (len(fldList)))
                Idx = 0
                for fldIdx in range(len(fldList)):
                    try:
                        fldByteSize = self.mapXpEaclV4KeyByteMask[eval(fldList[fldIdx])]
                    except KeyError:
                        fldByteSize = self.mapXpEaclV6KeyByteMask[eval(fldList[fldIdx])]
                    fldByteLen = bin(fldByteSize).count('1')
                    valueLst = []
                    maskLst = []
                    for valIdx in range(fldByteLen):
                        valX = uint8Arr_tp_getitem(xpEaclKeyMask_t_Ptr.value, Idx)
                        maskX = uint8Arr_tp_getitem(xpEaclKeyMask_t_Ptr.mask, Idx)
                        if(valX == 0 and maskX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(str(maskX))
                        elif(valX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                        elif(maskX == 0):
                            maskLst.append(str(maskX))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        else:
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        Idx = Idx + 1
                    if("IP" in fldList[fldIdx] or "MAC" in fldList[fldIdx]):
                        print(str('value'+str(fldIdx)) + '= %s' % (valueLst))
                        print(str('mask'+str(fldIdx)) + '= %s' % (maskLst))
                    else:
                        value = ''.join(reversed(valueLst))
                        mask = ''.join(reversed(maskLst))
                        print(str('value'+str(fldIdx)) + '= %d' % (int(value,16)))
                        print(str('mask'+str(fldIdx)) + '= %s' % (mask))
                pass
            delete_xpEaclKeyMask_tp(xpEaclKeyMask_t_Ptr)

    #/********************************************************************************/
    # command for xpsEaclGetNumOfValidEntries
    #/********************************************************************************/
    def do_eacl_get_num_of_valid_entries(self, arg):
        '''
         xpsEaclGetNumOfValidEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            noOfEntries_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclGetNumOfValidEntries(args[0],noOfEntries_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('noOfEntries = %d' % (uint32_tp_value(noOfEntries_Ptr_1)))
                pass
            delete_uint32_tp(noOfEntries_Ptr_1)

    #/********************************************************************************/
    # command for xpsEaclReadEntry
    #/********************************************************************************/
    def do_eacl_read_entry(self, arg):
        '''
         xpsEaclReadEntry: Enter [ devId,index]
        '''
        #,isValid,numFlds,type,enPktCmdUpd,enRsnCodeUpd,pktCmd,rsnCode,remarkDscp,remarkExp,remarkPcp,mirrorSsnUpd,mirrorSessionId,enCnt,enPolicer,dscpVal,espVal,pcpVal,counterId,policerId ]
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsEaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsEaclkeyFieldList_arr(1)
            xpsEaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            xpEaclkeyFieldList_t_Ptr = xpsEaclkeyFieldList_arr_getitem(fieldList_Ptr_pp, 0)
            xpsEaclData_t_Ptr = new_xpsEaclData_tp()
            #print('Input Arguments are, devId=%d, eaclType=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsEaclReadEntry(args[0],args[1],xpEaclkeyFieldList_t_Ptr,xpsEaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('enPktCmdUpd = %d' % (xpsEaclData_t_Ptr.enPktCmdUpd))
                print('enRsnCodeUpd = %d' % (xpsEaclData_t_Ptr.enRsnCodeUpd))
                print('pktCmd = %d' % (xpsEaclData_t_Ptr.pktCmd))
                print('rsnCode = %d' % (xpsEaclData_t_Ptr.rsnCode))
                print('remarkDscp = %d' % (xpsEaclData_t_Ptr.remarkDscp))
                print('remarkExp = %d' % (xpsEaclData_t_Ptr.remarkExp))
                print('remarkPcp = %d' % (xpsEaclData_t_Ptr.remarkPcp))
                print('mirrorSsnUpd = %d' % (xpsEaclData_t_Ptr.mirrorSsnUpd))
                print('mirrorSessionId = %d' % (xpsEaclData_t_Ptr.mirrorSessionId))
                print('enCnt = %d' % (xpsEaclData_t_Ptr.enCnt))
                print('enPolicer = %d' % (xpsEaclData_t_Ptr.enPolicer))
                print('dscpVal = %d' % (xpsEaclData_t_Ptr.dscpVal))
                print('espVal = %d' % (xpsEaclData_t_Ptr.espVal))
                print('pcpVal = %d' % (xpsEaclData_t_Ptr.pcpVal))
                print('counterId = %d' % (xpsEaclData_t_Ptr.counterId))
                print('policerId = %d' % (xpsEaclData_t_Ptr.policerId))
            delete_xpsEaclData_tp(xpsEaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsEaclReadKeyField
    #/********************************************************************************/
    def do_eacl_read_key_field(self, arg):
        '''
         xpsEaclReadKeyField: Enter [ devId,index,fldType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,fldType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            v4FldList = ['XP_EACL_KEY_TYPE_V4','XP_EACL_MAC_DA','XP_EACL_MAC_SA','XP_EACL_DIP_V4','XP_EACL_SIP_V4','XP_EACL_L4_DEST_PORT','XP_EACL_L4_SRC_PORT','XP_EACL_V4_ETHER_TYPE','XP_EACL_ICMP_MSG_TYPE','XP_EACL_PROTOCOL','XP_EACL_EGRESS_BD','XP_EACL_EGRESS_VIF','XP_EACL_VI','XP_EACL_DSCP_PCP_EXP','XP_EACL_TCP_FLAGS_RSNCODE']
            v6FldList = ['XP_EACL_KEY_TYPE_V6','XP_EACL_DIP_V6','XP_EACL_SIP_V6','XP_EACL_L4_V6_DEST_PORT','XP_EACL_L4_V6_SRC_PORT','XP_EACL_V6_ETHER_TYPE','XP_EACL_ICMP_V6_MSG_TYPE','XP_EACL_V6_PROTOCOL','XP_EACL_V6_EGRESS_BD','XP_EACL_V6_EGRESS_VIF','XP_EACL_V6_RSNCODE_DSCP','XP_EACL_V6_PCP_EXP']
            xpEaclKeyFieldId_t_ptr = xpEaclKeyFieldId_t()
            if(args[2] in v4FldList):
                xpEaclKeyFieldId_t_ptr.v4Fld = eval(args[2])
                byteSize = self.mapXpEaclV4KeyByteMask[eval(args[2])]
            elif(args[2] in v6FldList):
                xpEaclKeyFieldId_t_ptr.v6Fld = eval(args[2])
                byteSize = self.mapXpEaclV6KeyByteMask[eval(args[2])]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8_tp()
            fieldMask_Ptr_4 = new_uint8_tp()
            ret = xpsEaclReadKeyField(args[0],args[1],xpEaclKeyFieldId_t_ptr,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                value = []
                mask = []
                for valIdx in range(byteLen):
                    valX = uint8Arr_tp_getitem(fieldVal_Ptr_3, valIdx)
                    maskX = uint8Arr_tp_getitem(fieldMask_Ptr_4, valIdx)
                    value.append(str(valX))
                    mask.append(str(maskX))
                if("IP" in args[2] or "MAC" in args[2]):
                    print(str('value' + '= %s' % (value)))
                    print(str('mask' + '= %s' % (mask)))
                else:
                    value = ''.join(value)
                    mask = ''.join(mask)
                    print(str('value' + '= %d' % (int(value,16))))
                    print(str('mask' + '= %s' % (mask)))
                pass
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsEaclReadData
    #/********************************************************************************/
    def do_eacl_read_data(self, arg):
        '''
         xpsEaclReadData: Enter [ devId,index]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsEaclData_t_Ptr = new_xpsEaclData_tp()
            #print('Input Arguments are, devId=%d, index=%d ' % (args[0],args[1]))
            ret = xpsEaclReadData(args[0],args[1],xpsEaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enPktCmdUpd = %d' % (xpsEaclData_t_Ptr.enPktCmdUpd))
                print('enRsnCodeUpd = %d' % (xpsEaclData_t_Ptr.enRsnCodeUpd))
                print('pktCmd = %d' % (xpsEaclData_t_Ptr.pktCmd))
                print('rsnCode = %d' % (xpsEaclData_t_Ptr.rsnCode))
                print('remarkDscp = %d' % (xpsEaclData_t_Ptr.remarkDscp))
                print('remarkExp = %d' % (xpsEaclData_t_Ptr.remarkExp))
                print('remarkPcp = %d' % (xpsEaclData_t_Ptr.remarkPcp))
                print('mirrorSsnUpd = %d' % (xpsEaclData_t_Ptr.mirrorSsnUpd))
                print('mirrorSessionId = %d' % (xpsEaclData_t_Ptr.mirrorSessionId))
                print('enCnt = %d' % (xpsEaclData_t_Ptr.enCnt))
                print('enPolicer = %d' % (xpsEaclData_t_Ptr.enPolicer))
                print('dscpVal = %d' % (xpsEaclData_t_Ptr.dscpVal))
                print('espVal = %d' % (xpsEaclData_t_Ptr.espVal))
                print('pcpVal = %d' % (xpsEaclData_t_Ptr.pcpVal))
                print('counterId = %d' % (xpsEaclData_t_Ptr.counterId))
                print('policerId = %d' % (xpsEaclData_t_Ptr.policerId))
                pass
            delete_xpsEaclData_tp(xpsEaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsEaclEnable
    #/********************************************************************************/
    def do_eacl_enable(self, arg):
        '''
         xpsEaclEnable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclEnable(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEaclDisable
    #/********************************************************************************/
    def do_eacl_disable(self, arg):
        '''
         xpsEaclDisable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEaclDisable(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for iacl help 
    #/********************************************************************************/
    def do_eacl_help(self, arg):
        '''
         xpsEaclInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            print('Input Arguments are : Not required' % ())
            helpBuff = "================================ EACL_configGuide===================================\n \
            use help <CMD>, required input argument \n \
            1-Initialize the eacl module if not already done by the SDK \n \
            e.g eacl_init  \n \
                eacl_add_device 0 0 \n \
            2-Create table for EACL module \n \
            e.g eacl_create_table 0 1 1 200 \n \
            3-Define the key  \n \
            e.g eacl_define_key 0 1 4 XP_EACL_V4_TYPE XP_EACL_KEY_TYPE_V4,XP_EACL_PROTOCOL,XP_EACL_EGRESS_VIF,XP_EACL_DSCP_PCP_EXP \n \
            4-Write the key values into the HW \n \
            e.g eacl_write_key 0 1 1 4 XP_EACL_V4_TYPE XP_EACL_KEY_TYPE_V4,0,0 XP_EACL_PROTOCOL,6,0 XP_EACL_EGRESS_VIF,38,0 XP_EACL_DSCP_PCP_EXP,11,0 \n \
			At last append key want to be program in this formate 'fld,vlaue,mask'. numFlds and number of keys you append must match. \n \
            *For mask, \n \
            0 : to set all mask bit to 0000 0000 \n \
            1 : to set all mask bit to 1111 1111 \n \
            63 : to set particular bit mask i.e [ 63 in dec = 0011 1111] \n \
            5-Write rule data into the HW  \n \
            e.g eacl_write_data 0 1 1 1 0 202 0 0 0 1 1 0 0 0 0 0 0 0 \n \
            6-Enable Eacl\n \
            e.g eacl_enable 0\n \
            7-Delete table for EACL module for a acl type. \n \
            e.g eacl_delete_table 0  \n \
            Possible Keytypes: XP_EACL_V4_TYPE and XP_EACL_V6_TYPE \n \
            Possible fields in the IPV4 Key: XP_EACL_KEY_TYPE_V4,XP_EACL_MAC_DA,XP_EACL_MAC_SA,XP_EACL_DIP_V4,XP_EACL_SIP_V4,XP_EACL_L4_DEST_PORT,XP_EACL_L4_SRC_PORT, \n \
                                        XP_EACL_V4_ETHER_TYPE,XP_EACL_ICMP_MSG_TYPE,XP_EACL_PROTOCOL,XP_EACL_EGRESS_BD,XP_EACL_EGRESS_VIF,XP_EACL_VI,XP_EACL_DSCP_PCP_EXP, \n \
                                        XP_EACL_TCP_FLAGS_RSNCODE \n \
            Possible fields in the IPV6 Key: XP_EACL_KEY_TYPE_V6,XP_EACL_DIP_V6,XP_EACL_SIP_V6,XP_EACL_L4_V6_DEST_PORT,XP_EACL_L4_V6_SRC_PORT,XP_EACL_V6_ETHER_TYPE, \n \
                                        XP_EACL_ICMP_V6_MSG_TYPE,XP_EACL_V6_PROTOCOL,XP_EACL_V6_EGRESS_BD,XP_EACL_V6_EGRESS_VIF,XP_EACL_V6_RSNCODE_DSCP,XP_EACL_V6_PCP_EXP \n "
            print helpBuff.strip(' ')
            pass
    #/********************************************************************************/
    # command for xpsEaclSetCountingEnable
    #/********************************************************************************/
    def do_eacl_set_counting_enable(self, arg):
        '''
         xpsEaclSetCountingEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsEaclSetCountingEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsEaclUpdateDataField
    #/********************************************************************************/
    def do_eacl_update_data_field(self, arg):
        '''
         xpsEaclUpdateDataField: Enter [ devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpEaclDataUpdate_t_Ptr = new_xpEaclDataUpdate_tp() 
            xpEaclDataUpdate_t_Ptr.dataEnFlds = eval(args[2])
            xpEaclDataUpdate_t_Ptr.dataFlds = eval(args[4])
            xpEaclDataUpdate_t_Ptr.dataEnValue =int(args[3])
            xpEaclDataUpdate_t_Ptr.dataValue = int(args[5])
            #print('Input Arguments are, devId=%d, camIndex=%d, dataEnType =%d,dataEnValue=%d,dataType=%d dataValue=%d, ' % (args[0],args[1],xpEaclDataUpdate_t_Ptr.dataEnFlds,xpEaclDataUpdate_t_Ptr.dataEnValue,xpEaclDataUpdate_t_Ptr.dataFlds,xpEaclDataUpdate_t_Ptr.dataValue))
            ret = xpsEaclUpdateDataField(args[0],args[1],xpEaclDataUpdate_t_Ptr)
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


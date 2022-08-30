#!/usr/bin/env python
# cmdsGen.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell’s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

import sys
import time
import os
import re
import readline
import subprocess
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd

#/*********************************************************************************************************/
# import everything from buildTarget
#from buildTarget import *

#/*********************************************************************************************************/
# The class object for command utility
#/*********************************************************************************************************/
class runCmdUtility(Cmd):
    #/*****************************************************************************************************/
    # command to print the header
    #/*****************************************************************************************************/
    def printCmdHeader(self, arg):
        print('#!/usr/bin/env python')
        print('#  %s' % (arg))
        print('#')
        print('#/*******************************************************************************')
        print('#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *')
        print('#* subject to the limited use license agreement by and between Marvell and you, *')
        print('#* your employer or other entity on behalf of whom you act. In the absence of   *')
        print('#* such license agreement the following file is subject to Marvell’s standard   *')
        print('#* Limited Use License Agreement.                                               *')
        print('#*******************************************************************************/')
        print('#* @file %s' % (arg))
        print('#')
        print('')
        print('import sys')
        print('import time')
        print('import os')
        print('import re')
        print('import readline')
        print('')
        print('#/**********************************************************************************/')
        print('# import cmd2 package')
        print('#/**********************************************************************************/')
        print('dirname, filename = os.path.split(os.path.abspath(__file__))')
        print('sys.path.append(dirname + "/../cli")')
        print('sys.path.append(dirname + "/../../cli")')
        print('from cmd2x import Cmd')
        print('import xpShellGlobals')
        print('')
        print('#/**********************************************************************************/')
        print('# import everything from buildTarget')
        print('from buildTarget import *')
        print('')

    #/*****************************************************************************************************/
    # command to print the header
    #/*****************************************************************************************************/
    def isApiDef(self, line):
        sety = ';()'
        for c in sety:
            if c not in line:
                return 0
        if ':' in line:
            return 0
        if '~' in line:
            return 0
        return 1

    #/*****************************************************************************************************/
    # command to convert to camel case
    #/*****************************************************************************************************/
    def convert(self, name):
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
        return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


    #/*****************************************************************************************************/
    # command to extend the list within list
    #/*****************************************************************************************************/
    #olist[1:1] = nlist
    def lextend(self, nlist, idx):
        self[idx:idx] = nlist

    #/*****************************************************************************************************/
    # command to turn format
    #/*****************************************************************************************************/
    def rformat(self, strj, pfmt):
        rfmt = 'int(args[' + strj + '])'
        if pfmt == '%u':
            rfmt = 'eval(args[' + strj + '])'
        else:
            if pfmt == '0x%x':
                rfmt = 'int(args[' + strj + '], 16)'
            else:
                rfmt = 'int(args[' + strj + '])'
        return rfmt

    #/*****************************************************************************************************/
    # command to reformt the line
    #/*****************************************************************************************************/
    def reformat(self, line):
        if 'virtual' in line:
            line = line.replace("=", ";")
            line = line.replace("virtual", " ")

        line = line.replace("inline", " ")
        line = line.replace("unsigned", " ")
        line = line.replace("const", " ")
        line = line.replace(" struct", " ")

        while ' );' in line:
            line = line.replace(" );", ");")

        while ' ;' in line:
            line = line.replace(" ;", ";")

        while ' *' in line:
            line = line.replace(" *", "* ")

        while ' &' in line:
            line = line.replace(" &", "& ")

        while ' ,' in line:
            line = line.replace(" ,", ",")

        while ', ' in line:
            line = line.replace(", ", ",")

        line = '<'.join(line.split())
        line = line.replace(",<", "<")
        line = line.replace("(<", "<")
        line = line.replace("<(", "<")
        line = line.strip(";")
        line = line.strip(")")
        #line = line.strip("}")
        line = line.strip("{")

        while ';' in line:
            line = line.replace(";", "")

        return line

    #/*****************************************************************************************************/
    # command to check and skip line
    #/*****************************************************************************************************/
    def skipline(self, line):
        if line in ['\n', '\r\n']:
            return 1
        if '//' in line:
            return 1
        if '#' in line:
            return 1
        if 'nop' in line:
            return 1
        return 0


    #/*****************************************************************************************************/
    # find the file path
    #/*****************************************************************************************************/
    def findpath(self, path, file):
        for r,d,f in os.walk(path):
            for files in f:
                if files == file:
                    return os.path.join(r,files)
        return 'None'

    #/*****************************************************************************************************/
    # find the typedef struct
    #/*****************************************************************************************************/
    def findstruct(self, token, tlist, tnum):
        k = 0
        found = -1
        for k in range(0, tnum+1):
            if 'typedefenum' in tlist[k]:
                continue
            if '}' not in tlist[k]:
                continue
            tline = tlist[k].split('}')
            if token in tline[1]:
                #print('Hunt token1 = %s , %s' % (token1, tline))
                found = k
                break
        return found

    #/*****************************************************************************************************/
    # find the int array
    #/*****************************************************************************************************/
    def isIntArr(self, token):
        return 0

    #/*****************************************************************************************************/
    # find the char array
    #/*****************************************************************************************************/
    def isCharArr(self, token):
        if token == 'macAddr_t':
            return 6
        if token == 'ipv4Addr_t':
            return 4
        if token == 'ipv6Addr_t':
            return 16
        if token == 'compIpv6Addr_t':
            return 6
        if token == 'macAddrHigh_t':
            return 5
        #if token == 'macAddrLow_t':
        #    return 1
        return 0

    #/*****************************************************************************************************/
    # find the typedef enum
    #/*****************************************************************************************************/
    def findenum(self, token, tlist, tnum):
        k = 0
        found = -1
        for k in range(0, tnum+1):
            if 'typedefenum' not in tlist[k]:
                continue
            #print('HUNT token1 = %s , %s' % (token, tlist[k]))
            if token in tlist[k]:
                found = k
                break
        return found


#/*********************************************************************************************************/
#/* main program */
#/*********************************************************************************************************/

if __name__ == '__main__':
    #/*****************************************************************************************************/
    #Init/addDevice for Device manager and tableManager
    # Initialize only of we are running in standalone mode
    #init and redirect logger prints to console
    #/*****************************************************************************************************/
    inFile = 'infile.txt'
    instancex = ''

    if  len(sys.argv) > 1 :
        inFilex = sys.argv[1]
    else:
        print('Invalid input. Enter the input fileName with function definitions')
        sys.exit(0)

    filex = re.split('\/',inFilex)
    lenx = len(filex)
    inFile = filex[lenx-1]

    print('Input FileName = %s' % (inFile))
    className = inFile.split(".")
    xpMgr = inFile.replace("xps", "xp")
    xpMgr = xpMgr.strip(".h") + 'Mgr'
    outfile = className[0] + '.py'

    if  len(sys.argv) > 2 :
        instancex = sys.argv[2]

    ofx = sys.stdout
    fx = open(outfile,'a');
    sys.stdout = fx
    #hunt
    #sys.stdout = ofx

    tlist = [''] * 256
    tnum = -1
    tproc = 0

    tmap = [''] * 256
    tmnum = -1

    kutility = runCmdUtility()

    kutility.printCmdHeader(outfile)
    print('')
    print('#/**********************************************************************************/')
    print('# The class object for %s operations' % (className[0]))
    print('#/**********************************************************************************/')
    print('')
    #print('class %sObj(Cmd):' % (className[0]))
    firstCmd = 1
    prepath = inFilex.split("include")
    baseDir =  inFilex.split(os.path.dirname(inFilex).strip('../'))
    lineNum = -1
    line = ''
    orgline = ''
    skipinline = 0
    with open(inFilex) as f:
        for line in f:
            #print('line=%s' % (line))
            prevline = orgline
            orgline = line
            lineNum = lineNum + 1
            if line in ['\n', '\r\n']:
                continue

            if skipinline == 1 and '}' not in line:
                continue

            skipinline = 0

            if tproc == 1:
                if ';' in line:
                    tokenx = re.split(': | ;',line)
                    if '//' in tokenx[0] or '/*' in  tokenx[0]:
                        continue
                    if '(' in tokenx[0] and ')' in tokenx[0]:
                        continue
                    tlist[tnum] = tlist[tnum] + tokenx[0]

                if '}' in line:
                    tlist[tnum] = kutility.reformat(tlist[tnum])
                    #print('tHunt tlist[tnum] = %d : %s', (tnum, tlist[tnum]))
                    tproc = 0

            if 'typedef struct' in line:
                if 'std::' in line:
                    continue
                tnum = tnum+1
                tproc = 1
                continue

            if 'typedef enum' in line:
                if 'std::' in line:
                    continue
                tnum = tnum+1
                tproc = 1
                tlist[tnum] = tlist[tnum] + 'typedefenum'
                continue

            if 'typedef' in line:
                if 'std::' in line:
                    continue

            if 'typedef' in line and ';' in line:
                if 'std::' in line:
                    continue
                tmnum = tmnum + 1
                tmap[tmnum] = tmap[tmnum] + line
                continue

            #process include files for its typedefs
            if '#include' in line:
                sline = line.split()
                incfile = sline[1].strip('"|<|>')
                incfile =  kutility.findpath('./' + baseDir[0],incfile)

                #print('Hunt Found file path = %s' % (incfile))
                if incfile == 'None':
                    continue

                with open(incfile) as incf:
                    iprox = 0
                    for line in incf:

                        if 'typedef struct' in line:
                            if 'std::' in line:
                                continue
                            tnum = tnum+1
                            iprox = 1
                            continue

                        if 'typedef enum' in line:
                            if 'std::' in line:
                                continue
                            tnum = tnum+1
                            iprox = 1
                            tlist[tnum] = tlist[tnum] + 'typedefenum'
                            continue

                        if 'typedef' in line:
                            if 'std::' in line:
                                continue

                        if 'typedef' in line and ';' in line:
                            if 'std::' in line:
                                continue
                            tmnum = tmnum + 1
                            tmap[tmnum] = tmap[tmnum] + line
                            continue

                        if iprox == 0:
                            continue

                        if ';' in line:
                            tokenx = re.split(',|:|;',line)
                            if '//' in tokenx[0] or '/*' in  tokenx[0]:
                                continue
                            if '(' in tokenx[0] and ')' in tokenx[0]:
                                continue
                            tlist[tnum] = tlist[tnum] + tokenx[0]

                        if '}' in line:
                            tlist[tnum] = kutility.reformat(tlist[tnum])
                            #print('tHunt tlist[tnum] = %d : %s', (tnum, tlist[tnum]))
                            iprox = 0
                    continue

            if 'typedef' in line:
                continue

            if '//' in line:
                continue

            if '#' in line:
                continue

            if 'nop' in line:
                continue

            #if '{' in line:
            #    if '}' not in line:
            #        skipinline = 1
            #    splitline = re.split('{', line)
            #    line = splitline[0] + ';'

            if 'virtual' in line:
                line = line.replace("=", ";")
                line = line.replace("virtual", " ")

            #find the API lines
            isApi = kutility.isApiDef(line)

            #sometimes the API can be spread in two lines
            if isApi == 0:
                if kutility.isApiDef(prevline) == 1:
                    continue
                prevline = prevline.strip('\r\n')
                line1 = line.strip('\r\n')
                line = prevline+line1
                isApi = kutility.isApiDef(line)
                if isApi == 0:
                    continue

            line = kutility.reformat(line)

            #print('Newline = %s' % (line))
            tokens = re.split(',|;| |\(|\)|<|\n',line)
            filter(None, tokens)
            lenx = len(tokens)
            #for i in range(0, lenx):
                #print('HUNTy %d - %s' % (i, tokens[i]))

            if lenx < 3:
                continue;

            helpStr = ''
            fname = tokens[1]
            fxname = kutility.convert(fname)
            fxname = fxname.replace("xps_",'')
            if firstCmd == 1:
                print('class %sObj(Cmd):' % (className[0]))
                firstCmd = 0
                print('')
                print('    doc_header   = \'Available commands (type help <topic>):\'')
                print('    undoc_header = \'Utility commands\'')
                print('')
            print('    #/********************************************************************************/')
            print('    # command for %s' % (fname))
            print('    #/********************************************************************************/')
            print('    def do_%s(self, arg):' % (fxname))
            print('        \'\'\'')
            j=0
            numArgs = (lenx - 2) / 2

            #check if it is a get command
            isget = 0
            if 'get_' in fxname or 'read_' in fxname or '_get' in fxname or '_read' in fxname:
                isget = 1

            fmt = '#print(\'Input Arguments are'
            fmtv = '('
            xmtv = '('
            flv = [''] * 128
            retval = [''] * 128
            dlist = [''] * 128
            lfmt = [''] * 128
            dnum = 0
            rnum = 0
            rval = 0
            numPtr = 0
            addlen = 0
            inlen = 0

            if numArgs < 1 :
                helpStr = helpStr + ' void'
                fmt = fmt + ' : Not required'
                fmtv = '('
                xmtv = '('
                flv[j] = ''
            else:
                done1 = 0
                donex = 0
                ridx = 0
                for i in xrange(2, lenx, 2):
                    tfound = 0
                    k = 0
                    star = 0
                    isalist = 0
                    ischararr = 0
                    isintarr = 0
                    arrTypeNew = 'new_uint8Arr_tp'
                    arrTypeDel = 'delete_uint8Arr_tp'
                    arrTypeGet = 'uint8Arr_tp_getitem'
                    arrTypeSet = 'uint8Arr_tp_setitem'
                    numz = 2
                    newtoks = [''] * 32
                    token1 = tokens[i]
                    token0 = token1
                    if '*' in token0 or '&' in token0:
                        star = 1
                        token0 = token0.strip("*")
                        token0 = token0.strip("&")
                    t0 = 0
                    t1 = 0

                    if(lenx - 1 == i):
                        continue

                    #print('i=%d, tokens[i]=%s' % (i, tokens[i]))
                    token2 = tokens[i+1]
                    #print('i+1=%d, tokens[i+1]=%s' % (i+1, tokens[i+1]))
                    temp = token0
                    k = kutility.findstruct(token0, tlist, tnum)
                    if k == -1:
                        for l in range(0, tmnum+1):
                            #print('Huntz1 %s %s' % (token0, tmap[l]))
                            if token0 in tmap[l]:
                                temp1 = tmap[l].split()
                                temp = temp1[1]
                                #print('Huntz2 temp = %s' % temp)
                                k = kutility.findstruct(temp, tlist, tnum)
                                if k == -1 and token0 not in temp1[2]:
                                    temp = temp1[2]
                                    #print('Huntz3 temp = %s' % temp)
                                    k = kutility.findstruct(temp, tlist, tnum)
                                if k != -1:
                                    break

                    if k != -1:
                        #token0 = temp
                        tfound = 1

                    if tfound == 1:
                        tline = tlist[k].split('}')
                        #print('tline = %s' % (tline))
                        newtoks = re.split(',|;| |\(|\)|<|\n',tline[0])
                        numz = len(newtoks) - 1
                        addlen = (numz / 2)
                        inlen = addlen - 1
                    else:
                        newtoks[0] = token1
                        newtoks[1] = token2

                    #print('netwoks0 = %s' % newtoks[0])
                    #print('netwoks1 = %s' % newtoks[1])

                    for k in xrange(0, numz, 2):
                        ridx = ridx + 1
                        isalist = 0
                        if tfound == 1:
                            #print('k=%d, numz=%d' % (k,numz))
                            token1 = newtoks[k]
                            token2 = newtoks[k+1]

                        #check if the token is a list.
                        ischararr = kutility.isCharArr(token1)
                        isintarr = kutility.isIntArr(token1)

                        if ischararr > 0 or isintarr > 0:
                            #print('Hunty token1 = %s' % token1)
                            isalist = 1

                        if ischararr > 0:
                            arrTypeNew = 'new_' + token1 + 'p'
                            arrTypeDel = 'delete_' + token1 + 'p'
                            arrTypeGet =  token1 + 'p_getitem'
                            arrTypeSet =  token1 + 'p_setitem'

                        if isintarr > 0:
                            arrTypeNew = 'new_arrUint32'
                            arrTypeDel = 'delete_arrUint32'
                            arrTypeGet = 'arrUint32_getitem'
                            arrTypeSet = 'arrUint32_setitem'

                        #print("Hunt line = %s" % line)
                        pfmt = '%d'
                        #print('Hunt token1=%s, token2=%s' % (token1, token2))

                        # check for list
                        if '[' in token2:
                            isalist = 1
                            token2 = token2.split("[")[0]

                            if 'int8' in token1:
                                ischararr = 1
                                isintarr = 0
                                arrTypeNew = 'new_uint8Arr_tp'
                                arrTypeDel = 'delete_uint8Arr_tp'
                                arrTypeGet = 'uint8Arr_tp_getitem'
                                arrTypeSet = 'uint8Arr_tp_setitem'
                            else:
                                isintarr = 1
                                ischararr = 0
                                arrTypeNew = 'new_arrUint32'
                                arrTypeDel = 'delete_arrUint32'
                                arrTypeGet = 'arrUint32_getitem'
                                arrTypeSet = 'arrUint32_setitem'

                        if '*' in token1 or '&' in token1:
                            star = 1
                            pfmt = '%s'
                        else:
                            ltoken = token2.lower()
                            if token1 in 'uint64_t' or 'hex' in ltoken or 'addr' in ltoken or 'value' in ltoken or 'data' in ltoken:
                                pfmt = '0x%x'
                            else:
                                ckenum = token1
                                ckenum = ckenum.strip('_')
                                temp = token1
                                enumx = kutility.findenum(token1, tlist, tnum)
                                if enumx == -1:
                                    for l in range(0, tmnum+1):
                                        if token1 in tmap[l]:
                                            temp1 = tmap[l].split()
                                            temp = temp1[1]
                                            enumx = kutility.findenum(temp, tlist, tnum)
                                            if enumx == -1 and token1 not in temp1[2]:
                                                temp = temp1[2]
                                                enumx = kutility.findenum(temp, tlist, tnum)
                                            break

                                if enumx != -1:
                                    pfmt = '%u'
                                else:
                                    if not any(x.islower() for x in ckenum) or '_e' in token1:
                                        pfmt = '%u'
                                    else:
                                        pfmt = '%d'

                        lfmt[j] = pfmt
                        if lfmt[j] == '%u':
                            lfmt[j] = '%d'

                        # if it is a get command
                        #if isget == 1:
                        #    pfmt = '%s'

                        if pfmt != '%s':
                            if helpStr == '':
                                helpStr = helpStr + ' ' + token2
                            else:
                                helpStr = helpStr + ',' + token2
                            kfmt = pfmt
                            if kfmt == '%u':
                                kfmt = '%d'
                            if isalist == 1:
                                kfmt = '%s'
                            fmt = fmt + ', ' + token2 + '=' + kfmt

                        if pfmt != '%s' and tfound == 0:
                            if done1 > 0:
                                fmtv = fmtv + ','
                            if donex > 0:
                                xmtv = xmtv + ','
                            if isalist:
                                fmtv = fmtv + token2
                            else:
                                fmtv = fmtv + 'args[' + str(j) + ']'
                            xmtv = xmtv + 'args[' + str(j) + ']'
                            done1 = done1 + 1
                            donex = donex + 1
                        else:
                            if tfound == 1:
                                if t0 == 0:
                                    if done1 > 0:
                                        fmtv = fmtv + ','
                                    fmtv = fmtv + token0 + '_Ptr'
                                    done1 = done1 + 1
                                    t0 = 1
                            else:
                                if done1 > 0:
                                    fmtv = fmtv + ','
                                done1 = done1+1
                                fmtv = fmtv + token2 + '_Ptr_' + str(j)
                            if star == 0 or tfound == 1:
                                if donex > 0:
                                    xmtv = xmtv + ','
                                if tfound == 1:
                                    if isalist == 1:
                                        xmtv = xmtv + 'args[' + str(j) + ']'
                                    else:
                                        xmtv = xmtv + token0 + '_Ptr' + '.' + token2
                                else:
                                    xmtv = xmtv + 'args[' + str(j) + ']'
                                donex = donex + 1

                        ptype = ''
                        ppval = ''

                        if pfmt == '%s' or tfound == 1:
                            #if tokens[i] in ['uint8_t*', 'uint8_t&', 'char*', 'char&']:
                            #    flv[j] = 'args[' + str(j) + ']' + ' = ' + 'str(args[' + str(j) + '])'
                            #else:
                            ptype = token1
                            ptype = ptype.strip("*")
                            ptype = ptype.strip("&")

                            if tfound == 1:
                                if t1 == 0:
                                    if star == 0:
                                        ppval = token0 + '()'
                                    else:
                                        ppval = 'new_' + token0 + 'p()'
                                        if isalist == 1:
                                            dlist[dnum] = arrTypeDel + '(' + token0 + '_Ptr.' + token2 + ')'
                                            #print('Here dlist of dnum is %s' % dlist[dnum])
                                            dnum = dnum + 1
                                        dlist[dnum] = 'delete_' + token0 + 'p(' + token0 + '_Ptr' + ')'
                                        dnum = dnum + 1
                            else:
                                ppval = 'new_' + ptype + 'p()'
                                if isalist == 1:
                                    dlist[dnum] = arrTypeDel + '(' + token0 + '_Ptr.' + token2 + ')'
                                    print('Here is dlist of dnum is %s' % dlist[dnum])
                                    dnum = dnum + 1

                                dlist[dnum] = 'delete_' + ptype + 'p(' + token2 + '_Ptr_' + str(j) + ')'
                                dnum = dnum + 1

                            if tfound == 1:
                                if t1 == 0:
                                    flv[j] = token0 + '_Ptr' + ' = ' + ppval
                                    flv[j] = flv[j] + '\n            '
                                    t1 = 1

                            #initiliaztion of structure or pointer element. If is is list we allocate an
                            #array and set the lements
                            if isalist == 1:
                                if tfound == 1:
                                    rfmt = kutility.rformat(str(j), pfmt)
                                    flv[j] = flv[j] + '\n'
                                    flv[j] = flv[j] + '            ' + 'args[' + str(j) + '] = args[' + str(j) + '].replace(".",":").replace(",",":")\n'
                                    flv[j] = flv[j] + '            ' + 'postList = args[' + str(j) + '].strip(",").strip("\'").strip("]").strip("[").split(":")\n'
                                    flv[j] = flv[j] + '            ' + 'listLen = len(postList)\n'
                                    if '*' in token1:
                                        flv[j] = flv[j] + '            ' + token0 + '_Ptr.' + token2 + ' = ' + arrTypeNew + '(listLen+1)\n'
                                    flv[j] = flv[j] + '            ' + 'for ix in range(listLen-1, -1, -1):\n'
                                    if "ipv6" in token1 or "mac" in  token1:
                                        flv[j] = flv[j] + '            ' + '    ' +  token0 + '_Ptr.' + token2 + '[listLen - ix - 1] = int(postList[ix], 16)'
                                    else:
                                        flv[j] = flv[j] + '            ' + '    ' +  token0 + '_Ptr.' + token2 + '[listLen - ix - 1] = int(postList[ix])'

                                    #to print return values from the array
                                    if star == 1:
                                        retval[rnum] = 'print(\'' + token2 + ' = \'),\n'
                                        retval[rnum] = retval[rnum] + '                ' + 'for ix in range(0, listLen):\n'
                                        retval[rnum] = retval[rnum] + '                ' + '    print(\'%02x\' % (' + token0 + '_Ptr.' + token2 + '[ix])),\n'
                                        retval[rnum] = retval[rnum] + '                ' + '    if ix < (listLen-1):\n'
                                        retval[rnum] = retval[rnum] + '                ' + '        sys.stdout.write(\':\'),\n'
                                        retval[rnum] = retval[rnum] + '                ' + 'print(\'\''
                                        rnum = rnum + 1
                                else:
                                    flv[j] = flv[j] + 'postList = args[' + str(j) + '].strip(":").strip("\'").strip("]").strip("[").split(",")\n'
                                    flv[j] = flv[j] + '            listLen = len(postList)\n'
                                    if '*' in token1:
                                        flv[j] = flv[j] + '            ' + token2 + '_Ptr_' + str(j) + ' = ' + arrTypeNew + '(listLen+1)\n'
                                    flv[j] = flv[j] + '            ' + 'for ix in range(0, listLen):\n'
                                    flv[j] = flv[j] + '            ' + '    ' + arrTypeSet + token2 + '_Ptr_' + str(j) + ', ix, int(postList[ix], 16))'

                                if '*' in token1:
                                    dlist[dnum] = arrTypeDel + '(' + token0 + '_Ptr.' + token2 + ')'
                                    dnum = dnum + 1
                            else:
                                if tfound == 1:
                                    rfmt = kutility.rformat(str(j), pfmt)
                                    flv[j] = flv[j] + token0 + '_Ptr' + '.' + token2  + ' = '  + rfmt
                                    if star == 1:
                                        retval[rnum] = 'print(\'' + token2 + ' = %d\' % (' + token0 + '_Ptr.' + token2  + ')'
                                        rnum = rnum + 1
                                else:
                                    flv[j] = token2 + '_Ptr_' + str(j) + ' = ' + ppval

                            if star == 1:
                                rval = 1

                            # Value of return
                            if star == 1 and tfound == 0:
                                retval[rnum] = 'print(\'' + token2 + ' = %d\' % (' + ptype + 'p_value(' + token2 + '_Ptr_' + str(j) + '))'
                                #Added below lines to store return value in global variable named retVal 
                                if (("xpsInterfaceId_t" in ptype) or ("xpsMcL2InterfaceListId_t" in ptype) or ("xpsStp_t" in ptype) or ("xpsMcL3InterfaceListId_t" in ptype)):
                                    rnum = rnum + 1
                                    retval[rnum] = 'xpShellGlobals.cmdRetVal = ('+ ptype + 'p_value(' + token2 + '_Ptr_' + str(j) + ')'         
                                rnum = rnum + 1
                                      
                                numPtr = numPtr + 1
                                rnum = rnum + 1
                        elif isalist:
                                flv[j] = flv[j] + 'args[' + str(j) + '] = args[' + str(j) + '].replace(".", ":").replace(",", ":")\n'
                                flv[j] = flv[j] + '            ' + token2 + 'List = args[' + str(j) + '].strip("\'").strip("]").strip("[").split(":")\n'
                                flv[j] = flv[j] + '            listLen = len(' + token2 + 'List)\n'
                                flv[j] = flv[j] + '            ' + token2 + ' = ' + token1 + '()\n'
                                flv[j] = flv[j] + '            ' + 'for ix in range(listLen-1, -1, -1):\n'
                                if "ipv6" in token1 or "mac" in  token1:
                                    flv[j] = flv[j] + '            ' + '    ' + token2 + '[listLen - ix - 1]' + ' = int(' + token2 + 'List[ix], 16)'
                                else:
                                    flv[j] = flv[j] + '            ' + '    ' + token2 + '[listLen - ix - 1]' + ' = int(' + token2 + 'List[ix])'
                        else:
                            rfmt = kutility.rformat(str(j), pfmt)
                            flv[j] = 'args[' + str(j) + ']' + ' = ' + rfmt
                        j = j+1
            fmtv = fmtv + ')'
            xmtv = xmtv + ')'
            fmt = fmt + '\' % ' + xmtv + ')'
            print('         %s: Enter [%s ]' % (fname, helpStr))
            print('        \'\'\'')
            print('        args = re.split(\';| \',arg)')
            #print('        ret = 0')
            print('        numArgsReq = %d' % (numArgs-numPtr+inlen))
            print('        if  (numArgsReq > 0 and args[0] == \'\') or (len(args) < numArgsReq) :')
            print('            print(\'Invalid input, Enter [%s ]\')' % (helpStr))
            print('        else:')
            for i in range(0, numArgs+addlen):
                print('            %s' % flv[i])
            print('            %s' % fmt)
            print('            ret = %s%s%s' % (instancex, fname, fmtv))

            if (('int' in tokens[0]) or (tokens[0] in ('XP_STATUS', 'int', 'uin32_t', 'uint8_t','char*'))): #Adding 'char*'.it prints err portion aftr calling API which has char* return type
                if rval == 1:
                    print('            err = 0')
                print('            if ret != 0:')
                printStr = 'print(\'Return Value = %d\' % (ret))'
                print('                %s' % (printStr))
                if rval == 1:
                    print('                err = 1')
                if rval == 0:
                    print('            else:')
                    print('                pass')

            if rval == 1:
                print('            if err == 0:')
                for i in range(0, rnum):
                    if retval[i] != '' :
                        print('                %s)' % retval[i])
                print('                pass')

            for i in xrange(dnum-1, -1, -1):
                if dlist[i] == '':
                    continue
                print('            %s' % (dlist[i]))


    print('    #/*****************************************************************************************************/')
    print('    # sub-commands to display tables')
    print('    #/*****************************************************************************************************/')
    print('    def do_display_tables(self, s):')
    print('        \'Display Table commands\'')
    print('        i = displayTableCmds()')
    print('        i.prompt = self.prompt[:-1]+\':displayTables)\'')
    print('        while True:')
    print('            try:')
    print('                i.cmdloop()')
    print('                break')
    print('            except (ValueError, IndexError), e:')
    print('                print \'Invalid input format, check help <command> or ? for usage\'')
    print('                continue')
    print('        del i')

    sys.stdout = ofx
    #/********************************************************************************************************************************************/
    #/ This section is applicable for xpLinkMgr.h file only.
    #/ The script lmAutoGenLinkMgr.py should be kept in cli/ dir where cmdsGen.py is kept
    #/ job of below section : 1. It adds range option for portNum/serdesId in xpLinkMgr 
    #/                        2. It adds check of entered port range is valid and inited or not in commands where port range option is available
    #/********************************************************************************************************************************************/
    if(className[0] == 'xpLinkMgr'):
        cmd = 'python autoGenLinkMgr.py && mv xpLinkMgrOut.py xpLinkMgr.py'
        subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
    #/*******************************************************************************************************************************************/
    # done with the command gen
    print('\nCmd class object for %s is generated in the file %s.py\n' % (className[0], className[0]))

#/*********************************************************************************************************/
#/* End of main program */
#/*********************************************************************************************************/



# coding=utf-8
import imp
import sys
import os
import testList
import testListFeature
import re
import glob
from datetime import datetime

# from scapy.all import *
try:
    import scapy.all as t_scapy
except IOError:
    print("Caught Exception IOError")
from threading import Thread
from time import sleep
from collections import defaultdict

try:
    from pexpect import pxssh as pxssh
except:
    print("pxssh is not installed, if you want to run apptest on Hw and testScript method please install it")
import time
import pdb
import sys

varDict = {}
TESTS_PREFIXES = ["saiCrm", "saiFdb", "saiL3", "saiMtu", "saiQos", "saiEverflow", "saiAclIngr", "saiAclEgr", "saiMac",
                  "saiPort", "saiCopp", "saiDecap", "saiSpan", "saiErspan", "saiVxlan", "saiHash", "saiSample"]



class SshConnection(object):
    def __init__(self, hostname, username, password):
        self.hostname = hostname
        self.username = username
        self.password = password
        try:
            self.ssh_conn = self.ssh_conn(self.hostname, self.username, self.password)
            self.ssh_conn.PROMPT = "[#>$]"
            self.ssh_conn.timeout = 5000
        except Exception as err:
            print("Connection to %s failed" % self.hostname)

    # To establish connection using ssh
    def ssh_conn(self, hostname, username, password):
        try:
            print("Connecting to", hostname)
            self.ssh = pxssh.pxssh()
            print(hostname, username, password)  # original_prompt='apc>', login_timeout=30, auto_prompt_reset=False
            self.ssh.login(hostname, username, password, original_prompt="[#>$]", login_timeout=5000,
                           auto_prompt_reset=False)
            print("Connected")

        except pxssh.ExceptionPxssh as error:
            print("Login failed")
            print(str(error))
            self.ssh = False
        return self.ssh

    # To apply command on ssh prompt, which is connected using ssh_conn methode

    def cmd(self, command, allowPrint=True, sleep=False, killApptest=False):
        prompt_repeat = 0
        # print self.ssh_conn.timeout
        retries = self.ssh_conn.timeout
        tmpbuf = ''
        # self.ssh_conn()
        self.ssh_conn.buffer = ''
        # print(self.ssh_conn.timeout)
        self.ssh_conn.sendline(command)
        time.sleep(0.02)
        if command.find("exit") != -1:
            return self.ssh_conn.before
        if sleep:
            time.sleep(20)
        if not self.ssh_conn.prompt(timeout=1):
            while self.ssh_conn.prompt(timeout=1) is not True and prompt_repeat < retries:
                if (self.ssh_conn.buffer != None):
                    tmpbuf = tmpbuf + self.ssh_conn.buffer
                self.ssh_conn.buffer = ''
                prompt_repeat += 1
            if prompt_repeat == retries:
                print("Failed to detect prompt %s for %s times" % (self.ssh_conn.PROMPT, retries))
                print(tmpbuf)
                if killApptest:
                    print("=" * 100)
                    print(
                            "Failed to detect prompt %s for %s times for %s cmd" % (
                        self.ssh_conn.PROMPT, retries, command))
                    print("=" * 100)
                    cleanup()
                    sys.exit()
                return False
            else:
                # print(prompt_repeat)
                # print("return from else: prompt_repeat == retries")
                return tmpbuf
        else:
            # if(debug == "ON"):
            tmpbuf = self.ssh_conn.before

            if allowPrint:
                print(tmpbuf)
            return tmpbuf

    def set_prompt_for_expect(prompt):
        self.ssh_conn.PROMPT = prompt

    def close(self):
        try:
            self.ssh_conn.close()
            print("connection to %s closed" % self.hostname)
        except Exception as err:
            print(err)


class FileLogger(object):
    def __init__(self, log_dir, log_name):
        self.terminal = sys.stdout
        if not os.path.exists(log_dir) or not os.path.isdir(log_dir):
            os.makedirs(log_dir)
        ext = os.path.splitext(log_name)[1]
        if ext is None or ext.strip() == '':
            log_name = '{}.txt'.format(log_name)
        self.log_path = os.path.abspath(os.path.join(log_dir, log_name))
        self.log = open(self.log_path, 'a')
        print("Initiated log file: {}".format(self.log_path))

    def write(self, msg):
        self.terminal.write(msg)
        self.log.write(msg)

    def flush(self):
        self.log.close()


class RunData(object):
    def __init__(self, total=0):
        self.total = total
        self.passed = 0
        self.failed = 0
        self.executed = 0
        self.tests = []
        self.failed_testcase_list = []

    def add_result(self, test_name, status_str, run_time):
        test_prefix = next((x for x in TESTS_PREFIXES if x in test_name), None)
        if test_prefix is not None:
            test_name = test_name.replace(test_prefix, test_prefix + '_')

        if 'pass' in status_str.lower():
            self.passed = self.passed + 1
        else:
            self.failed = self.failed + 1
            self.failed_testcase_list.append(test_name)
        self.executed = self.executed + 1
        log_path = sys.stdout.log_path if isinstance(sys.stdout, FileLogger) else None
        self.tests.append({'name': test_name, 'status': status_str, 'time': int(round(run_time)), "log": log_path})
        print("Test {0} {1} ({2}){3}{3}".format(test_name, status_str, run_time, os.linesep))

    def get_json(self):
        import json
        return json.dumps(self.tests, indent=4, sort_keys=True)

    def print_summary(self):
        print('--------------- TEST SUMMARY ---------------')
        print('***Total Test Cases***\t\t' + str(self.total))
        print('***Test Cases Executed***\t' + str(self.executed))
        print('***Test Cases Passed***\t\t' + str(self.passed))
        print('***Test Cases Failed***\t\t' + str(self.failed))
        print('--------------------------------------------')
        if self.failed != 0:
            print('--------------- FAILED TEST CASES ---------------\n')
            for x in range(len(self.failed_testcase_list)):
                print('\t' + str(self.failed_testcase_list[x]))


def runApp(xpType, testUnit):
    if xpType == "sai":
        run_app = "./dist/xpSaiApp -g FALCON128 -u"
    elif xpType == "sai-m0":
        run_app = "./dist/xpSaiApp -g ALDB2B -u"
    elif xpType == "sai-aldrin2":
        run_app = "./dist/xpSaiApp -g ALDB2B -u"
    elif xpType == "ebof":
        run_app = "./dist/xpSaiApp -g FALCONEBOF -u"
    elif xpType == "sai-ac5x":
        run_app = "./dist/xpSaiApp -g AC5XRD -u"
    elif xpType == "sai-fc2T":
        run_app = "./dist/xpSaiApp -g F2T80x25G -u"
    elif xpType == "sai-fs":
        run_app = "./dist/xpSaiApp -g AC3XFS -u"
    elif xpType == "sai-fl":
        run_app = "./dist/xpSaiApp -g ALDRIN2XLFL -u"
    else:
        print("Invalid argument for xptype. Taking sai as default")
        run_app = "./dist/xpSaiApp -g FALCON128 -u"

    cdXdkDir = 'cd ' + pathToXdk
    testUnit.cmd(cdXdkDir)
    testUnit.ssh_conn.PROMPT = "(\(xpShell\))"
    testUnit.cmd(run_app, True, True)


def usage():
    print("python apptest.py <ip> <username> <passwd> <xdk_path> <xptype> <testType> <testName> <enableLog>")
    print("    xptype   = sai/xps")
    print("    testType = feature/sanity")
    print("    testName = all(if all testcase to be executed)/<feature prefix>/<name of testcase>")
    print("example : python apptest.py 127.0.0.1 <username> <password> <sai_cpss repo-path> sai sanity all 1\n")
    cleanup()
    sys.exit()


def macCountersCheck(port, test, mod):
    egressPort = mod.tcParams['egressPort']
    ingressPort = mod.tcParams['ingressPort']
    i = 0
    pos = 0
    pktCnt = '0x1'
    if 'packetCount' in mod.tcParams.keys():
        pktCnt = '0x' + str(mod.tcParams['packetCount'])
    rx_res = []
    tx_res = []
    rx = ['RxUC', 'RxMC', 'RxBC']
    tx = ['TxUC', 'TxMC', 'TxBC']
    var = rx + tx
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('mac', enableLog)
    command = 'mac_get_counter_stats 0 ' + port
    counterstr = test.cmd(command, enableLog)
    counterstr = counterstr.strip()
    counterstr = counterstr.replace(' ', '')
    tempPort = port
    for word in var:
        pos = counterstr.find(word) + len(word)
        if (i < len(rx)):
            rx_res.append(counterstr[pos: pos + 3])
        else:
            tx_res.append(counterstr[pos: pos + 3])
        i = i + 1
    print(tx_res)
    print(rx_res)
    if pktCnt not in rx_res:
        print("packets not correctly received at the Ingress Port")
        return False

    for port in egressPort:
        if (port == tempPort):
            print("Mac counters matched")
            return True
        command = 'mac_get_counter_stats 0 ' + port
        counterstr = test.cmd(command, enableLog)
        counterstr = counterstr.strip()
        counterstr = counterstr.replace(' ', '')
        i = 0
        rx_res = []
        tx_res = []
        for word in var:
            pos = counterstr.find(word) + len(word)
            if (i < len(rx)):
                rx_res.append(counterstr[pos: pos + 3])
            else:
                tx_res.append(counterstr[pos: pos + 3])
            i = i + 1
        print(rx_res)
        print(tx_res)
        if pktCnt not in tx_res:
            print("packets not correctly egressed out of the Egress Port")
            return False
    print("Mac counters matched")
    return True


def getCpuRxCounter(test):
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('packetdrv', enableLog)
    test.cmd("packet_driver_get_pkt_rx_tx_statistics 0", enableLog)
    countStr = test.cmd("packet_driver_get_pkt_rx_tx_statistics 0", enableLog)
    countStr = countStr.replace('\r\n', '')
    countStr = "".join(countStr.split())
    subStr1 = "RxCount"
    subStr2 = "TxCount"
    idx1 = countStr.find(subStr1)
    idx2 = countStr.find(subStr2)
    substr = countStr[idx1:idx2]
    countStr = substr
    idx1 = countStr.find(",")
    substr = countStr[11:idx1]
    rx_val = int(substr)
    return rx_val


def cpuCountersCheck(test, pktCnt):
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('packetdrv', enableLog)
    test.cmd("packet_driver_get_pkt_rx_tx_statistics 0", enableLog)
    counterstr = test.cmd("packet_driver_get_pkt_rx_tx_statistics 0", enableLog)
    counterstr = counterstr.replace('\r\n', '')
    counterstr = "".join(counterstr.split())
    findStr = "RxCounters=" + str(pktCnt)
    if enableLog:
        print("****CPU counters  ***")
        print(counterstr)
        print("*******")
    pos = counterstr.find(findStr)
    if pos != -1:
        return True
    else:
        return False


def comparestr(output, expect1):
    # print "Comparing strings : " + "output = " + output + "expected = " + expect1
    p = output
    p = p.replace(' ', '')
    p = "".join(p.split())
    q = expect1
    q = q.replace(' ', '')
    q = "".join(q.split())
    if p.find(q) != -1:
        return True
    else:
        return False


def flush_all(test, mod):
    flushlocal(test, mod)
    flush(test, mod)


def flushlocal(test, mod):
    egressPort = mod.tcParams['egressPort']
    ingressPort = mod.tcParams['ingressPort']
    test.cmd("home", enableLog)
    test.cmd('xps', enableLog)
    test.cmd('packetdrv', enableLog)

    # for port in egressPort:
    # if port != '':
    # capture = 'packet_driver_trap_ingress_traffic 0 ' + port + ' 0'
    # test.cmd(capture, enableLog)
    test.cmd('back', enableLog)
    test.cmd('mac', enableLog)
    # for port in ingressPort:
    # loopback = 'mac_mac_loopback 0 ' + port + ' 2 0'
    # test.cmd(loopback, enableLog)


def flush(test, mod):
    flushlist = mod.tcFlushStr.split('\n')
    cmnd_otpt = ''
    # print(flushlist)
    for z in flushlist[:-1]:
        if enableLog:
            print(z)
        cmnd_otpt = ''
        cmnd_otpt = test.cmd(z, False)
        cmnd_otpt = cmnd_otpt.replace(z, "")
        if enableLog:
            print(cmnd_otpt)
        if cmnd_otpt.find("invalid") != -1:
            print("invalid input" + cmnd_otpt)
            test.cmd('home', False)
            cleanup()
        if cmnd_otpt.find("error") != -1:
            print("command error" + cmnd_otpt)
            test.cmd('home', False)
            cleanup()


def get_first_port(test):
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('port', enableLog)
    port = test.cmd('port_get_first', enableLog)
    port = "".join(port.split())
    port = port.split('=')[1]
    return port


def get_next_port(test, port):
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('port', enableLog)
    command = 'port_get_next ' + str(port)
    port = test.cmd(command, enableLog)
    port = "".join(port.split())
    port = port.split('=')[1]
    return port


def get_max_port(test):
    test.cmd('home', enableLog)
    test.cmd('xps', enableLog)
    test.cmd('port', enableLog)
    command = 'port_get_max_num 0'
    port = test.cmd(command, enableLog)
    port = "".join(port.split())
    port = port.split('=')[1]
    return port


def get_sai_port_list(test, max_ports):
    test.cmd('home', enableLog)
    test.cmd('sai', enableLog)
    test.cmd('switch', enableLog)
    command = 'sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_PORT_LIST ' + str(max_ports)
    port_list = test.cmd(command, enableLog)
    port_list = port_list.split()
    sai_port_list = port_list[-int(max_ports):]
    return sai_port_list


# def sniffPacket(x):
#    global pkts
#    pkts = defaultdict(list)
#    try:
#       pct = pkts[x.sniffed_on].append(x)
#    except:
#       print("Caught Exception in sniffing")
#    return pct

def implementation(filename, testname, test_path, test):
    global run_data
    global numEgrPort
    global pkts
    start_time = datetime.now()
    acl_counter_check = False
    rif_counter_check = False
    acl_counter_list = []
    rif_counter_list = []
    acl_counter_count = []
    rif_counter_count = []
    enable_loop = False
    Counter = 1
    mismatch = 0
    print("Loading test {} from {}".format(testname, test_path))
    mod = imp.load_source(filename, test_path)
    cmdlist = mod.tcProgramStr.split('\n')
    cmdlist = [i for i in cmdlist if i.strip('')]  # Removing empty commands
    cmnd_otpt = ''
    if 'sleep_time' in mod.tcParams.keys():
        sleepTime = mod.tcParams['sleep_time']
        print "Recieved sleep time of " + str(sleepTime) + " seconds"
        sleep(sleepTime)
        print "sleep step completed"
    if 'acl_counter' in mod.tcParams:
        if mod.tcParams['acl_counter'] != '':
            acl_counter_check = True
    if 'rif_counter' in mod.tcParams:
        if mod.tcParams['rif_counter'] != '':
            rif_counter_check = True
    if 'counter' in mod.tcParams:
        if mod.tcParams['counter'] != '':
            enable_loop = True
    if enable_loop:
        if mod.tcParams['counter'] == 'max_ports':
            Counter = get_max_port(test)
            port = get_first_port(test)
            if devType == 'Falcon':
                Counter = int(Counter) - 1
        elif mod.tcParams['counter'] == 'sai_ports':
            Counter = get_max_port(test)
            sai_ports = get_sai_port_list(test, Counter)
            if devType == 'Falcon':
                Counter = int(Counter) - 1
        else:
            Counter = mod.tcParams['counter']
    k = 0
    while k < int(Counter):
        for z in cmdlist:
            if enable_loop:
                if z.find('$counter') != -1:
                    if mod.tcParams['counter'] == 'max_ports':
                        z = z.replace("$counter", str(port))
                    if mod.tcParams['counter'] == 'sai_ports':
                        z = z.replace("$counter", str(sai_ports[k]))
            if enableLog:
                print(z)
            cmnd_otpt = test.cmd(z, False)
            cmnd_otpt = "\n".join(cmnd_otpt.split("\n")[1:])
            if enableLog:
                print(cmnd_otpt)
            if '>' in z:
                varKey = z.split('>')[-1].strip('\n').strip(' ')
                varDict[varKey] = cmnd_otpt.strip("\n").strip("\r").strip(" ")
            if cmnd_otpt.find("Invalid") != -1:
                print("invalid input" + cmnd_otpt)
                test.cmd('home', False)
                test.cmd('exit 3', False)
                cleanup()
                exit(0)

            if acl_counter_check == True:  # saving the counter obj Id returned for future use
                if z.find('sai_create_acl_counter') != -1:
                    for counter in mod.tcParams['acl_counter']:
                        if z.find(counter) != -1:
                            for output in cmnd_otpt.split():
                                if output.isdigit():
                                    acl_counter_list.append(output)
            if rif_counter_check == True:  # saving the counter obj Id returned for future use
                if z.find('sai_create_router_interface') != -1:
                    for counter in mod.tcParams['rif_counter']:
                        if z.find(counter) != -1:
                            for output in cmnd_otpt.split():
                                if output.isdigit() and int(output) >= 1688849860263936:
                                    rif_counter_list.append(output)
        # print "Last command output : " + cmnd_otpt
        if acl_counter_check == True:  # checking counters before sending traffic
            test.cmd('home', enableLog)
            test.cmd('sai', enableLog)
            test.cmd('acl', enableLog)
            for counter in acl_counter_list:
                cmd = 'sai_get_acl_counter_attribute ' + str(counter) + ' SAI_ACL_COUNTER_ATTR_PACKETS 1'
                cmd_otpt = test.cmd(cmd, enableLog)
                one_count = 0
                for output in cmd_otpt.split():
                    if output.isdigit() and output != str(counter):
                        if one_count >= 1:
                            acl_counter_count.append(output)
                            break
                        if output == '1':
                            one_count += 1
                if cmd_otpt.find("Invalid") != -1:
                    print("invalid input" + cmd)
                    test.cmd('home', False)
                    test.cmd('exit 3', False)
                    cleanup()
                    exit(0)
        if rif_counter_check is True:  # checking counters before sending traffic
            test.cmd('home', enableLog)
            test.cmd('sai', enableLog)
            test.cmd('routerinterface', enableLog)
            pktAction = mod.tcParams['pktAction']
            i = 1
            if pktAction == 'DROP':
                i = 5
            for counter in rif_counter_list:
                cmd = 'sai_get_router_interface_stats ' + str(counter) + ' ' + str(i)
                cmd_otpt = test.cmd(cmd, enableLog)
                one_count = 0
                i += 2
                op = cmd_otpt.split()
                output = op[-1]
                if output.isdigit() and output != str(counter):
                    rif_counter_count.append(output)
                if cmd_otpt.find("Invalid") != -1:
                    print("invalid input" + cmd)
                    test.cmd('home', False)
                    test.cmd('exit 3', False)
                    cleanup()
                    exit(0)

        if mod.tcParams['ingressPacket'] != '':
            test.cmd('home', enableLog)
            test.cmd('xps', enableLog)
            test.cmd('packetdrv', enableLog)
            ingressPort = mod.tcParams['ingressPort']
            egressPort = mod.tcParams['egressPort']
            numEgrPort = numEgrPort + len(egressPort)
            pktAction = mod.tcParams['pktAction']
            pktCount = 1
            if 'packetCount' in mod.tcParams.keys():
                pktCount = mod.tcParams['packetCount']
            # test.cmd(("packet_driver_receive " + str(len(ingressPort))), enableLog)
            test.cmd("packet_driver_receive 10", enableLog)
            for port in egressPort:
                if port != '':
                    # capture = 'packet_driver_trap_ingress_traffic 0 ' + port + ' 1'
                    # test.cmd(capture, enableLog)
                    # test.cmd(capture, enableLog)
                    test.cmd('home', enableLog)
                    test.cmd('xps', enableLog)
                    test.cmd('mac', enableLog)
                    port_num = 281474976710656 + int(port)
                    port_enable_get = 'mac_get_link_status 0 ' + str(port)
                    cmd_otpt = test.cmd(port_enable_get, enableLog)
                    op = cmd_otpt.split()
                    print str(op)[1:-1]
                    output = op[-1]
                    if output.isdigit() and output != str(port_num):
                        if output != '1':
                            port_enable = 'sai_set_port_attribute ' + str(port_num) + ' SAI_PORT_ATTR_ADMIN_STATE 1'
                            test.cmd('home', enableLog)
                            test.cmd('sai', enableLog)
                            test.cmd('port', enableLog)
                            test.cmd(port_enable, enableLog)
                            index =  0
                            for x in range(30):
                                sleep(0.400)
                                test.cmd('home', enableLog)
                                test.cmd('xps', enableLog)
                                test.cmd('mac', enableLog)
                                cmd_otpt2 = test.cmd(port_enable_get, enableLog)
                                opt = cmd_otpt2.split()
                                print str(opt)[1:-1]
                                output2 = opt[-1]
                                if output2.isdigit() and output2 != str(port_num):
                                    if output2 == '1':
                                        break
                                index = index +1
                            print "Iterations: " + str(index)

                    test.cmd('home', enableLog)
                    test.cmd('xps', enableLog)
                    test.cmd('packetdrv', enableLog)
                    clear_cpu_stats = 'packet_driver_get_pkt_rx_tx_statistics 0'
                    result = test.cmd(clear_cpu_stats, enableLog)
                    clear = 'mac_stat_counter_reset 0 ' + port
                    test.cmd('back', enableLog)
                    test.cmd('mac', enableLog)
                    test.cmd(clear, enableLog)
                    test.cmd('back', enableLog)
                    test.cmd('packetdrv', enableLog)
            test.cmd('back', enableLog)
            test.cmd('mac', enableLog)

            for port in ingressPort:
                # loopback = 'mac_mac_loopback 0 ' + port + ' 2 1'
                clear = 'mac_stat_counter_reset 0 ' + port
                test.cmd(clear, enableLog)
                test.cmd('home', enableLog)
                test.cmd('xps', enableLog)
                test.cmd('mac', enableLog)
                port_num = 281474976710656 + int(port)
                port_enable_get = 'mac_get_link_status 0 ' + str(port)
                cmd_otpt = test.cmd(port_enable_get, enableLog)
                op = cmd_otpt.split()
                print str(op)[1:-1]
                output = op[-1]
                if output.isdigit() and output != str(port_num):
                    if output != '1':
                        port_enable = 'sai_set_port_attribute ' + str(port_num) + ' SAI_PORT_ATTR_ADMIN_STATE 1'
                        test.cmd('home', enableLog)
                        test.cmd('sai', enableLog)
                        test.cmd('port', enableLog)
                        test.cmd(port_enable, enableLog)
                        index = 0
                        for x in range(30):
                            sleep(0.400)
                            test.cmd('home', enableLog)
                            test.cmd('xps', enableLog)
                            test.cmd('mac', enableLog)
                            cmd_otpt2 = test.cmd(port_enable_get, enableLog)
                            opt = cmd_otpt2.split()
                            print str(opt)[1:-1]
                            output2 = opt[-1]
                            if output2.isdigit() and output2 != str(port_num):
                                if output2 == '1':
                                    break
                            index = index + 1
                        print "Iterations: " + str(index)
                # test.cmd(loopback, enableLog)
                # test.cmd(loopback, enableLog)
                # test.cmd('back', enableLog)
                # test.cmd('packetdrv', enableLog)
                # sendCmd = 'packet_driver_send_pcap_with_dsa 0 ' + port + ' 0 ' + 'apptest/pcaps/' + mod.tcParams[
                # 'ingressPcap']
                test.cmd('home', enableLog)
                # test.cmd('shell sudo scapy', enableLog)
                # sniff(iface=["tap0","tap1","tap2"],prn= lambda x: pkts[x.sniffed_on].append(x),timeout=4)
                rxVal = getCpuRxCounter(test)
                iTap = mod.tcParams['ingressTapIntf']
                eTap = mod.tcParams['egressTapIntf']
                pkts = defaultdict(list)
                if (eTap):
                    etapList = list(eTap)
                    t = t_scapy.AsyncSniffer(iface=etapList, prn=lambda x: pkts[x.sniffed_on].append(x), timeout=30)
                    # t = scapy_t.AsyncSniffer(iface=etapList,prn= sniffPacket(x),timeout=30)
                    t.start()
                p = mod.packet_info
                t_scapy.sendp(p, iface=iTap, count=pktCount)
                sleep(4)
                if (eTap):
                    t.join()
                print(pkts)
                # cmnd_otpt = test.cmd(sendCmd, enableLog)
                # time.sleep(4)
                # cmnd_otpt = test.cmd("\r", enableLog)
                # cmnd_otpt = test.cmd("\r", enableLog)
                if enableLog:
                    print("packets sniffed after AsyncSniffer stopped")
                    print(pkts)
                try:
                    etaps = pkts.keys()
                except:
                    print("Caught exception ignored!!!")

                if acl_counter_check == True:  # checking counter incremented after sending traffic
                    test.cmd('home', enableLog)
                    test.cmd('sai', enableLog)
                    test.cmd('acl', enableLog)
                    sleep(3)
                    index = 0
                    for counter in acl_counter_list:
                        one_count = 0
                        cmd = 'sai_get_acl_counter_attribute ' + str(counter) + ' SAI_ACL_COUNTER_ATTR_PACKETS 1'
                        cmd_otpt = test.cmd(cmd, enableLog)
                        for output in cmd_otpt.split():
                            if output.isdigit() and output != str(counter):
                                if one_count >= 1:
                                    if int(output) - int(acl_counter_count[index]) != pktCount:
                                        run_data.add_result(testname, 'Failed in Acl Counter check',
                                                            (datetime.now() - start_time).total_seconds())
                                        flush_all(test, mod)
                                        return
                                    break
                                if output == '1':
                                    one_count += 1
                        index += 1
                    if enableLog == True:
                        print "Acl Counters incremented as expected !"
                    test.cmd('home', enableLog)
                if rif_counter_check == True:  # checking counter incremented after sending traffic
                    test.cmd('home', enableLog)
                    test.cmd('sai', enableLog)
                    test.cmd('routerinterface', enableLog)
                    sleep(3)
                    index = 0
                    i = 1
                    if pktAction == 'DROP':
                        i = 5
                    # rif_counter_list should hold item in fixed order[ing_pass_rif, egr_pass_rif_id]
                    for counter in rif_counter_list:
                        cmd = 'sai_get_router_interface_stats ' + str(counter) + ' ' + str(i)
                        cmd_otpt = test.cmd(cmd, enableLog)
                        op = cmd_otpt.split()
                        output = op[-1]
                        if output.isdigit() and output != str(counter):
                            if i == 3:
                                if pktAction == 'DROP':
                                    index += 1
                                    i += 2
                                    continue
                            if ((int(output) - int(rif_counter_count[index])) < pktCount):
                                run_data.add_result(testname, 'Failed in RIF Counter check',
                                                    (datetime.now() - start_time).total_seconds())
                                print int(output)
                                print int(rif_counter_count[index])
                                print pktCount
                                flush_all(test, mod)
                                return
                        index += 1
                        i += 2
                    if enableLog == True:
                        print "RIF Counters incremented as expected !"
                    test.cmd('home', enableLog)

                if pktAction == 'DROP':
                    ret = macCountersCheck(port, test, mod)
                    if ret is False:
                        flush_all(test, mod)
                        run_data.add_result(testname, 'Passed', (datetime.now() - start_time).total_seconds())
                        return
                    else:
                        print("Mac counters matched in DROP case implies drop is not happening")
                        flush_all(test, mod)
                        run_data.add_result(testname, 'Failed', (datetime.now() - start_time).total_seconds())
                        return
                # TODO FORWARD AND MIRROR case should be handled for another cpu code check
                elif pktAction == 'FORWARD' or pktAction == 'FORWARD AND MIRROR':
                    i = 1
                    mismatch = 0
                    while i <= mod.tcParams['count']:
                        name = 'expect' + str(i)
                        if len(etaps) and len(eTap):
                            eTapData = "'" + str(etaps[i - 1]) + "'" + ":" + str(pkts[eTap[i - 1]])
                            print eTapData
                            print mod.expectedData[name]
                            ret = comparestr(eTapData, mod.expectedData[name])
                            if ret is False:
                                print('expected data ' + str(i) + ' mismatched')
                                mismatch = mismatch + 1
                            else:
                                print('expected data ' + str(i) + ' matched')
                        else:
                            print('No packets are captured by the sniffer')
                        i = i + 1
                    if mismatch == 0:
                        # ret = cpuCountersCheck(test, numEgrPort)
                        ret = macCountersCheck(port, test, mod)
                        if ret is True:
                            flush_all(test, mod)
                            run_data.add_result(testname, 'Passed', (datetime.now() - start_time).total_seconds())
                            return
                        else:
                            print("MAC counters mismatched")
                            flush_all(test, mod)
                            run_data.add_result(testname, 'Failed', (datetime.now() - start_time).total_seconds())
                            return
                    else:
                        flush_all(test, mod)
                        run_data.add_result(testname, 'Failed', (datetime.now() - start_time).total_seconds())
                        return
                elif pktAction == 'TRAP':
                    pktCount = pktCount + rxVal
                    ret = cpuCountersCheck(test, pktCount)
                    if ret is True:
                        flush_all(test, mod)
                        run_data.add_result(testname, 'Passed', (datetime.now() - start_time).total_seconds())
                        return
                    else:
                        print("CPU counters not Updated")
                        flush_all(test, mod)
                        run_data.add_result(testname, 'Failed', (datetime.now() - start_time).total_seconds())
                        return
        else:
            i = 1
            while i <= mod.tcParams['count']:
                name = 'expect' + str(i)
                if '$' in mod.expectedData[name]:
                    varList = re.findall(r'[$]\w+', mod.expectedData[name])
                    for var in varList:
                        mod.expectedData[name] = mod.expectedData[name].replace(var, varDict[var.strip('$')])
                    mod.expectedData[name] = str(eval(mod.expectedData[name]))
                ret = comparestr(cmnd_otpt, mod.expectedData[name])
                if ret == False:
                    print('expected data ' + str(i) + ' mismatched')
                    mismatch = mismatch + 1
                else:
                    print('expected data ' + str(i) + ' matched')
                i = i + 1

        k = k + 1
        if enable_loop:
            if mod.tcParams['counter'] == 'max_ports':
                port = get_next_port(test, port)

    flush(test, mod)
    status_str = 'Passed' if mismatch == 0 else 'Failed'
    run_data.add_result(testname, status_str, (datetime.now() - start_time).total_seconds())


def main():
    # Consider using python built-in argparse functionality
    argmnts = sys.argv[1:]  # ip username passwd app testName xdk_path enableLog are mandatory argument.
    if len(argmnts) == 0:
        usage()
    elif argmnts[0] in "help":
        usage()
    elif len(argmnts) < 8:
        usage()
    else:
        global testName
        global xpType
        global pathToXdk
        global tmpbuf
        global ingressPort
        global egressPort
        global enableLog
        global numEgrPort
        global run_data
        global pkts
        global devType
        global LOGS_DIR
        if argmnts[4] == "sai-m0":
            devType = "M0"
        elif argmnts[4] == "sai-aldrin2":
            devType = "Aldrin2"
        elif argmnts[4] == "ebof":
            devType = "ebof"
        elif argmnts[4] == "sai-ac5x":
            devType = "AC5X"
        elif argmnts[4] == "sai-fs":
            devType = "AC3XFS"
        elif argmnts[4] == "sai-fl":
            devType = "ALDRIN2XLFL"
        else:
            devType = "Falcon"
        LOGS_DIR = os.path.join("TestResults",
                                datetime.now().strftime(argmnts[4] + "-" + devType + "-%m.%d.%Y_%H-%M-%S"))
        print("Run results location: {}".format(os.path.abspath(LOGS_DIR)))
        print(argmnts)
        ip = argmnts[0]
        username = argmnts[1]
        passwd = argmnts[2]
        test = SshConnection(ip, username, passwd)
        pathToXdk = argmnts[3]
        if argmnts[7] == '1':
            enableLog = True
        else:
            enableLog = False
        numEgrPort = 0

        runApp(argmnts[4], test)

        if argmnts[5] == "feature":
            if devType == "Falcon":
                test_list = testListFeature.featureRegressionUT
            elif devType == "ebof":
                test_list = testListFeature.ebof_featureRegressionUT
            elif devType == "AC5X":
                test_list = testListFeature.ac5x_featureRegressionUT
            elif devType == "AC3XFS":
                test_list = testListFeature.fujitsu_featureRegressionUT
            elif devType == "ALDRIN2XLFL":
                test_list = testListFeature.fujitsu_featureRegressionUT
            else:
                test_list = testListFeature.m0_featureRegressionUT
        elif argmnts[5] == "sanity":
            test_list = testList.regressionUT
        else:
            print("Invalid argument for testType. Taking sanity as default")
            test_list = testList.regressionUT

        run_data = RunData(len(test_list))

        if argmnts[6] != 'all':
            test_list = sorted(glob.glob("testCases/{}*.py".format(argmnts[6])))
            test_list = [tc.split('/')[1] for tc in test_list]
            test_list = [tc.split('.')[0] for tc in test_list]

        if len(test_list) == 0:
            print("No test to run. Test list empty.")
            cleanup()
            sys.exit()

        # set the initial log level to DEFAULT
        test.cmd('home', enableLog)
        test.cmd('log', enableLog)
        test.cmd('set_log_level 2', enableLog)

        for test_name in test_list:
            filename = "{}.py".format(test_name)
            test_path = "testCases{}{}".format(os.sep, filename)
            # redirect all output to log per-test
            original_out = sys.stdout
            sys.stdout = FileLogger(LOGS_DIR, test_name)
            implementation(filename, test_name, test_path, test)
            sys.stdout = original_out


def post_run_actions():
    cleanup()
    write_results()


def write_results():
    global LOGS_DIR
    if LOGS_DIR is not None:
        with open(os.path.join(LOGS_DIR, "results.json"), 'w') as f:
            print("Results recorded to {}".format(os.path.abspath(f.name)))
            f.write(run_data.get_json())
    else:
        print("Unable to write results - logs dir is missing.")
    run_data.print_summary()


def cleanup():
    logs_dir = os.path.join("TestResults")
    CMD = "chmod -R a+rwx " + logs_dir
    os.system(CMD)
    CMD = "find . -name '*.pyc' | xargs chmod a+rwx "
    os.system(CMD)


LOGS_DIR = None

if __name__ == "__main__":
    try:
        main()
    finally:
        post_run_actions()


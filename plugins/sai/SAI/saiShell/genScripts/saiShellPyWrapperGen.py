#########################################################################
#									
# The following script generates Python wrappers required for SAI-SHELL 
# This scripts handles only SAI CREATE APIs ,the rest are handled by 
# shellCmdsGen.py. Generation of SET APIs requires the UNION type 
# sai_attribute_t to be taken care.This was not being handled by shellCmdsGen.py 
# so a new script was required for generation of SAI Wrappers
#########################################################################

import cPickle

f = open("db_file.txt","r") #db_file.txt has the extracted data-base from XML of sai-header-files
db = cPickle.load(f)
f.close()

f = open("list_struct_db.txt","r")
lb = cPickle.load(f)
f.close()

f = open("union_db.txt","r")
ub = cPickle.load(f)
f.close()

f = open("api_structures.txt","r")
apistruct = cPickle.load(f)
f.close()

f = open("api_enums.txt","r")
apienums = cPickle.load(f)
f.close()


def gen_create_func():

    filelist = db

    for eachfile in filelist.keys():
	#print eachfile
        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py','w') #Open the existing py file in append mode

        printCmdHeader(fns, file_name+'.py')

	fns.write("\n\n#/**********************************************************************************/")
	fns.write('\n# The class object for '+file_name+' operations')
	fns.write('\n#/**********************************************************************************/')
	fns.write("\n\nclass "+file_name+'Obj(Cmd):')	

	fns.write("\n\n    doc_header   = 'Available commands (type help <topic>):'")
	fns.write("\n    undoc_header = 'Utility commands'")

        objctlist = filelist[eachfile]

        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
	    param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            attr_list = objctmodules['attr_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0
            
            #TODO Handling list types(array types), mac , ip6 

            if 'CREATE' in fn_list.keys():
                create = fn_list['CREATE']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
		num_of_args = 0
                index = 0

                fns.write("\n\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_create_"+eachobjct                         )
                fns.write("\n"+"    #/*********************************************************/")

                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
                fns.write("\n\n"+" "*8+"''' sai_create_"+eachobjct+" '''\n")
                fns.write("\n"+" "*8+"if saiShellGlobals.createMode == 0:")
		gen_create_fns_body(fns,eachfile,file_name,objctlist,eachobjct,'default')
                fns.write("\n"+" "*8+"else:")
		gen_create_fns_body(fns,eachfile,file_name,objctlist,eachobjct,'custom')

def gen_create_fns_body(fns,eachfile,file_name,objctlist,eachobjct,case):

            objctmodules = objctlist[eachobjct]
            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            attr_list = objctmodules['attr_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0


            if 'CREATE' in fn_list.keys():
                create = fn_list['CREATE']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                num_of_args = 0
                index = 0


                num_of_param = create['numparam']
                                                                                                                                        
                #There are few objects which does not have an Object-id associated with them like Fdb, Route, Neighbor
                #All these objects are identified by their entries in the hardware like fdb_entry, route_entry, neighbor_entry, etc.. 
                #so these ogjects need to be handled uniquely

		list_size = 0
		fn_call = ''
                if eachfile == 'saifdb':#FdbEntry does not have an Object_id so to set an attribute user has to enter the fdb-entry so expansion of struct fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sairoute':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, vr_id, ip_addr_family, ip_addr , ip_mask,'
                      num_of_args = num_of_args+5

                elif eachfile == 'saineighbor':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, rif_id, ip_addr_family, ip_addr,'
                      num_of_args = num_of_args+4

                elif eachfile == 'saimcastfdb':#McastFdbEntry does not have an Object_id so to set an attribute user has to enter the mcast-fdb-entry so expansion of struct mcast-fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sail2mc':#L2McEntry does not have an Object_id so to set an attribute user has to enter the l2mc-entry so expansion of struct l2mc-entry
                      string = string+' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6

                elif eachfile == 'saiipmc':#IPMcEntry does not have an Object_id so to set an attribute user has to enter the ipmc-entry so expansion of struct ipmc-entry
                      string = string+' switch_id, vr_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6

                else:#If the file is none of the above find the primary arguments from the data base
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+' '+param_list[x][2]+','
			    attr_type_list.append(param_list[x][1])
			    attr_name_list.append(param_list[x][2])
			    list_size = list_size+1
                            num_of_args = num_of_args+1

		for attr_id in attr_list.keys():
                    supportflag = True
                    with open('../../xpSai/util/xpSaiNotSupported.txt','r') as ns:
                        for line in ns:
                            line = line.replace(' ','').replace(',','').replace(';','')
                            line = line.strip("/\n")
                            line.strip("/\t")
                            if (attr_id in line) and (line in attr_id):
                                supportflag = False
                                break
                    if supportflag:
                        attr_proprty = attr_list[attr_id]

			if ((case=='default')and((attr_proprty['mandatory_on_create'] == 'true') or (attr_proprty['mandatory_on_create'] == 'false' and attr_proprty['valid_for_create']== 'true' and attr_proprty['valid_for_set'] == 'false') or ('saiShellFdb' == file_name and attr_proprty['valid_for_create'] == 'true')))or ((case == 'custom')and(attr_proprty['valid_for_create'] == 'true')):
			    if 'sai_ip_address_t' in attr_proprty['attrib_type'][0]:
				attr_type_list.append(attr_proprty['attrib_type'][0])
				attr_name_list.append('ip_addr_family')
				string = string+' ip_addr_family,'
				num_of_args = num_of_args+1
				list_size = list_size+1

			    attr_type_list.append(attr_proprty['attrib_type'][0])
			    attr_name_list.append(attr_proprty['attrib_name'])
			    string = string+' '+ attr_proprty['attrib_name']+','
			    num_of_args = num_of_args+1
			    list_size = list_size+1
                if eachobjct == "acl_entry" and case == 'default':
                    attr_name_list.append('SAI_ACL_ENTRY_ATTRIBUTE_PRIORITY')
                    attr_type_list.append('sai_uint32_t')
                    string = string+' '+' priority'+','
                    num_of_args = num_of_args+1
                    list_size = list_size+1
		string = string.replace('*','')
		string = string[:-1]
                #fns.write("\n\n\t"+" "*4+"'''"+" "*4+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")
	
		#This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\t'+" "*4+'varStr = '+"'"+string+"'")
                fns.write('\n\t'+" "*4+'varList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\t'+" "*4+'optArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')

                fns.write("\n\t"+" "*4+"args = ['0']*len(varList)"+"\n\t"+" "*4+"totNumArgs = len(varList)")
                fns.write("\n\n\t"+" "*4+"argsList = re.split(' ',arg)")
                fns.write("\n\t"+" "*4+"args = argsList")

                fns.write("\n\t"+" "*4+"if (totNumArgs > 0 and argsList[0] == ''):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\t"+" "*4+"else:")
                if case == "custom" and eachobjct == "acl_entry":
                    fns.write("\n\t\t"+" "*4+"nameList = args[1::2]")
                    fns.write("\n\t\t"+" "*4+"dataList = args[2::2]")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0,len(nameList),1):")
                    fns.write("\n\t\t\t"+" "*4+"if \'-\' not in nameList[ix]:")
                    fns.write("\n\t\t\t\t"+" "*4+"print \'Invalid Input\'")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0,len(dataList),1):")
                    fns.write("\n\t\t\t"+" "*4+"if \'-\' in dataList[ix]:")
                    fns.write("\n\t\t\t\t"+" "*4+"print \'Invalid Input\'")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0,len(dataList),1):")
                    fns.write("\n\t\t\t"+" "*4+"nameList[ix] = nameList[ix].replace('-','')")
                    fns.write("\n\t\t"+" "*4+"flag = 0")

		#Need to parse the input arguments  differently for fdb, route, neighbor etc ..
                if eachfile == 'saifdb':
                    fns.write("\n\n\t\t"+" "*4+"sai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()")
                    fns.write("\n\t\t"+" "*4+"sai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+" "*4+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"sai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\t"+" "*4+"args[2]=int(args[2])")
                    fns.write("\n\t\t"+" "*4+"sai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]")
		    arg_index = 3
    
                elif eachfile == 'sairoute':
                    fns.write("\n\t\t"+" "*4+"ip_addr_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+" "*4+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t" +" "*4+ "base = 10")
                    fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                    fns.write("\n\t\t\t" +" "*4+ "base = 16")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"ip_addr_ptr.addr[ix] = int(postList[ix], base)")

                    fns.write("\n\t\t"+" "*4+"ip_mask_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+" "*4+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t" +" "*4+ "base = 10")
                    fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                    fns.write("\n\t\t\t" +" "*4+ "base = 16")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"ip_mask_ptr.addr[ix] = int(postList[ix], base)")
		    arg_index = 5

		elif eachfile == 'saineighbor':
                    fns.write("\n\t\t"+" "*4+"ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\t"+" "*4+"ip_addr_ptr.addr_family = eval(args[2])")
                    fns.write("\n\t\t"+" "*4+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t" +" "*4+ "base = 10")
                    fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                    fns.write("\n\t\t\t" +" "*4+ "base = 16")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
		    arg_index = 4

                elif eachfile == 'saimcastfdb':
                    fns.write("\n\n\t\t"+" "*4+"sai_mcast_fdb_entry_ptr = new_sai_mcast_fdb_entry_tp()")
                    fns.write("\n\t\t"+" "*4+"sai_mcast_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+" "*4+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"sai_mcast_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\t"+" "*4+"args[2]=int(args[2])")
                    fns.write("\n\t\t"+" "*4+"sai_mcast_fdb_entry_ptr.bv_id = args[2]")
                    arg_index = 3

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fns.write("\n\t\t"+" "*4+"src_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\t"+" "*4+"src_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+" "*4+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t" +" "*4+ "base = 10")
                    fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                    fns.write("\n\t\t\t" +" "*4+ "base = 16")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"src_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")

                    fns.write("\n\t\t"+" "*4+"dst_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\t"+" "*4+"dst_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+" "*4+'args[5] = args[5].replace(".",":").replace(",",":")')
                    text = 'postList = args[5].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+" "*4+text)
                    fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                    fns.write("\n\t\t" +" "*4+ "base = 10")
                    fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                    fns.write("\n\t\t\t" +" "*4+ "base = 16")
                    fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\t"+" "*4+"dst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
                    arg_index = 6
		elif case == "default" and eachobjct == "policer":
                    fns.write("\n\t\t"+" "*4+"color_source = \'SAI_POLICER_COLOR_SOURCE_AWARE\'")
                    fns.write("\n\t\t"+" "*4+"if len(args) >= totNumArgs:")
                    fns.write("\n\t\t\t"+" "*4+"color_source = args[3]")
		    arg_index = 0
		else:
		    arg_index = 0

		list_index = 0
		while arg_index < num_of_args and list_index < list_size:
		    #print list_index
		    if case == "custom" and eachobjct == "acl_entry":
		        for attrId in attr_list.keys():
		            attr_acl_proprty = attr_list[attrId]
		            if attr_acl_proprty['attrib_name'] == args[arg_index]:
		                break
		    if attr_type_list[list_index] in ub.keys():

                            #MAC data type
                            if  ub[attr_type_list[list_index]] == "mac":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
				fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr = new_shell_macp()')
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
				fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index]+'_ptr.mac[ix] =  int(postList[ix], 16)') 
				fn_call = fn_call+attr_name_list[list_index]+'_ptr,'
                            #IP6 data type
                            elif ub[attr_type_list[list_index]] == "ip6":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
				fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr = new_shell_ip6p()')
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index]+'_ptr.ip6[ix] = int(postList[ix])')
				fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            elif ub[attr_type_list[list_index]] == "ip4":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"postList = postList[::-1]")
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                                fns.write("\n\t\t"+" "*4+"x =0")
				fns.write("\n\t\t"+" "*4+"for ix in range(listLen):")
                                fns.write(" \n\t\t\t"+" "*4+"x = (x*256)+int(postList[ix])")
				fn_call = fn_call+' int(x),'

                            elif 'qos_map' in attr_type_list[list_index]:                               
                                fns.write("\n\t\t"+" "*4+"listLen = 0")
                                fns.write("\n\t\t"+" "*4+"postList = []")
                                fns.write("\n\t\t"+" "*4+"nameList = []")
                                fns.write("\n\t\t"+" "*4+"count = 0")
                                fns.write("\n\t\t"+" "*4+'if args['+str(arg_index)+'] != "[]":')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+')'+'.split(":")'
                                fns.write("\n\t\t\t"+" "*4+text)
                                fns.write("\n\t\t\t"+" "*4+"listLen = len(postList)")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr = new_'+attr_type_list[list_index]+'p()')
                                delete_string = delete_string+"\n\t\t"+" "*4+'delete_'+attr_type_list[list_index]+'p('+attr_name_list[list_index]+'_ptr)'
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr.count = listLen')
                                fns.write("\n\t\t"+" "*4+"list_ptr_arr = new_arr_"+attr_type_list[list_index].replace('_list_t','')+'(listLen)')
                                delete_string = delete_string+"\n\t\t"+" "*4+"delete_arr_"+attr_type_list[list_index].replace('_list_t','')+'(list_ptr_arr)'
                                fns.write("\n\t\t"+" "*4+'if ("SAI_QOS_MAP_TYPE" and "TO") not in args['+str(arg_index + 1)+']:')
                                fns.write("\n\t\t\t"+" "*4+'print " %s is Invalid argument" % (args['+str(arg_index + 1)+'])')
                                fns.write("\n\t\t\t"+" "*4+'return')
                                fns.write("\n\t\t"+" "*4+'nameList = args['+str(arg_index + 1)+'].replace("SAI_QOS_MAP_TYPE_","").replace("_","").lower().split("to")')
                                fns.write("\n\t\t"+" "*4+"nameDict = {'queue':'queue_index', 'pfcpriority':'prio', 'prioritygroup':'pg'}")
                                fns.write("\n\t\t"+" "*4+"for ix in range(0,len(nameList),1):")
                                fns.write("\n\t\t\t"+" "*4+"nameList[ix] = nameList[ix].split('and')")
                                fns.write("\n\t\t"+" "*4+"for ix in range(0,len(postList),1):")
                                fns.write("\n\t\t\t"+" "*4+"postList[ix] = postList[ix].split(',')")
                                fns.write("\n\t\t"+" "*4+"for ix in range(0,listLen,1):")
                                fns.write("\n\t\t\t"+" "*4+"if(len(nameList[0] + nameList[1]) != len(postList[ix])):")
                                fns.write("\n\t\t\t\t"+" "*4+'print " %s is Invalid argument" % (args['+str(arg_index)+'])')
                                fns.write("\n\t\t\t\t"+" "*4+'return')
                                fns.write("\n\t\t\t"+" "*4+"list_ptr = "+"arr_"+attr_type_list[list_index+1].replace('_type_t','')+'_getitem'+"(list_ptr_arr, ix)")
                                fns.write("\n\t\t\t"+" "*4+"for ix1 in range(0,len(postList[ix]),1):")
                                fns.write("\n\t\t\t\t"+" "*4+"if ix1 < len(nameList[0]):")
                                fns.write("\n\t\t\t\t\t"+" "*4+"for key,value in nameDict.items():")
                                fns.write("\n\t\t\t\t\t\t"+" "*4+"if nameList[0][ix1] == key:")
                                fns.write("\n\t\t\t\t\t\t\t"+" "*4+"nameList[0][ix1] = value")
                                fns.write("\n\t\t\t\t\t"+" "*4+"exec(\"list_ptr.key.\" + nameList[0][ix1] + \" = \"+postList[ix][ix1])")
                                fns.write("\n\t\t\t\t\t"+" "*4+"count += 1")
                                fns.write("\n\t\t\t\t"+" "*4+"else:")
                                fns.write("\n\t\t\t\t\t"+" "*4+"for key,value in nameDict.items():")
                                fns.write("\n\t\t\t\t\t\t"+" "*4+"if nameList[1][ix1 - count] == key:")
                                fns.write("\n\t\t\t\t\t\t\t"+" "*4+"nameList[1][ix1 - count] = value")
                                fns.write("\n\t\t\t\t\t"+" "*4+"exec(\"list_ptr.value.\" + nameList[1][ix1 - count] + \" = \"+postList[ix][ix1])")
                                fns.write("\n\t\t\t"+" "*4+"count = 0")
                                fns.write("\n\t\t\t"+" "*4+"arr_"+attr_type_list[list_index+1].replace('_type_t','')+'_setitem'+"(list_ptr_arr, ix, list_ptr)")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr.list = list_ptr_arr')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            elif '_list_t' in attr_type_list[list_index]:
                                fns.write("\n\t\t"+" "*4+"listLen = 0")
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+')')
                                fns.write("\n\t\t"+" "*4+'if args['+str(arg_index)+'] != "[]":')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t\t"+" "*4+text)
                                fns.write("\n\t\t\t"+" "*4+"listLen = len(postList)")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr = new_'+attr_type_list[list_index]+'p()')
                                delete_string = delete_string+"\n\t\t"+" "*4+'delete_'+attr_type_list[list_index]+'p('+attr_name_list[list_index]+'_ptr)'
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr.count = listLen')
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+"_list_ptr = new_arr_"+attr_type_list[list_index].replace('_list_t','')+'(listLen)')
                                delete_string = delete_string+"\n\t\t"+" "*4+"delete_arr_"+attr_type_list[list_index].replace('_list_t','')+'('+ attr_name_list[list_index]+'_list_ptr)'
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+"arr_"+attr_type_list[list_index].replace('_list_t','')+'_setitem'+'('+attr_name_list[list_index]+"_list_ptr, ix, int(postList[ix]))")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr.list = '+attr_name_list[list_index]+'_list_ptr')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            elif ub[attr_type_list[list_index]] == "booldata"  and (case != 'custom' and eachobjct != 'acl_entry'):
                                fn_call = fn_call+'int(args['+str(arg_index)+']) ,'

                            elif ub[attr_type_list[list_index]] == "ipaddr":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index+1)+'] = args['+str(arg_index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
				fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr = new_sai_ip_address_tp()')
                                delete_string = delete_string+"\n\t\t"+" "*4+'delete_sai_ip_address_tp('+attr_name_list[list_index+1]+'_ptr)'
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr_family = eval(args['+str(arg_index)+'])')
                                fns.write("\n\t\t" +" "*4+ "base = 10")
                                fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                                fns.write("\n\t\t\t" +" "*4+ "base = 16")
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr.ip6[ix] = int(postList[ix], base)')
				fn_call = fn_call+attr_name_list[list_index+1]+'_ptr,'
                                arg_index= arg_index+1
                                list_index= list_index+1

			    elif ub[attr_type_list[list_index]] == "chardata":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList[0])")
				fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+'_ptr = new_shell_stringp()')
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index]+"_ptr.name[ix] = postList[0][ix]")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index]+"_ptr.name[listLen] = \'\\0\'")
				delete_string = delete_string+"\n\t\t"+" "*4+"delete_"+'shell_stringp('+attr_name_list[list_index]+"_ptr)"
				fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            else:
                                if case == "custom" and eachobjct == "acl_entry":
                                    if args[arg_index] == 'switch_id':
                                        fn_call = fn_call+'int(args['+str(arg_index)+']),'
                                    else:
                                        if ub[attr_acl_proprty['attrib_type'][0]] == "aclfield":
                                            fns.write("\n\t\t"+" "*4+'acl_ptr_'+str(arg_index)+' = new_shell_acl_field_tp()')
                                            fns.write("\n\t\t"+" "*4+'for ix in range(0,len(nameList),1):')
                                            fns.write("\n\t\t\t"+" "*4+'if \"'+args[arg_index]+'\" == nameList[ix]:')
                                            fns.write("\n\t\t\t\t"+" "*4+"flag = 1")
                                            fns.write("\n\t\t\t\t"+" "*4+"valList = dataList[ix].split(',')")
                                            fns.write("\n\t\t\t\t"+" "*4+'for ix in range(0,len(valList),1):')
                                            fns.write("\n\t\t\t\t\t"+" "*4+"valList[ix] = valList[ix].replace('[','').replace(']','').replace(\"'\",'')")
                                            gen_acl_creat_api_body(fns,ub,attr_acl_proprty, arg_index, ub[attr_acl_proprty['attrib_type'][0]], 'valList')
                                            fns.write("\n\t\t"+" "*4+"if flag == 0:")
                                            fns.write("\n\t\t\t"+" "*4+'acl_ptr_'+str(arg_index)+'.enable = False')
                                            fns.write("\n\t\t"+" "*4+"flag = 0")
                                            delete_string = delete_string + "\n\t\t"+" "*4+"delete_shell_acl_field_tp(acl_ptr_"+str(arg_index)+")"
                                            fn_call = fn_call+'acl_ptr_'+str(arg_index)+','
                                        elif ub[attr_acl_proprty['attrib_type'][0]] == "aclaction":
                                            fns.write("\n\t\t"+" "*4+'acl_ptr_'+str(arg_index)+' = new_shell_acl_action_tp()')
                                            fns.write("\n\t\t"+" "*4+'for ix in range(0,len(nameList),1):')
                                            fns.write("\n\t\t\t"+" "*4+'if \"'+args[arg_index]+'\" == nameList[ix]:')
                                            fns.write("\n\t\t\t\t"+" "*4+"flag = 1")
                                            fns.write("\n\t\t\t\t"+" "*4+"valList = dataList[ix].split(',')")
                                            fns.write("\n\t\t\t\t"+" "*4+'for ix in range(0,len(valList),1):')
                                            fns.write("\n\t\t\t\t\t"+" "*4+"valList[ix] = valList[ix].replace('[','').replace(']','').replace(\"'\",'')")
                                            gen_acl_creat_api_body(fns,ub,attr_acl_proprty, arg_index, ub[attr_acl_proprty['attrib_type'][0]], 'valList')
                                            fns.write("\n\t\t"+" "*4+"if flag == 0:")
                                            fns.write("\n\t\t\t"+" "*4+'acl_ptr_'+str(arg_index)+'.enable = False')
                                            fns.write("\n\t\t"+" "*4+"flag = 0")
                                            delete_string = delete_string + "\n\t\t"+" "*4+"delete_shell_acl_action_tp(acl_ptr_"+str(arg_index)+")"
                                            fn_call = fn_call+'acl_ptr_'+str(arg_index)+','
                                        else:
					    if args[arg_index] == "admin_state":
					        fns.write("\n\t\t"+" "*4+'acl_ptr_'+str(arg_index)+' = new_sai_attribute_value_tp()')
                                                fns.write("\n\t\t"+" "*4+'acl_ptr_'+str(arg_index)+'.'+ub[attr_acl_proprty['attrib_type'][0]]+'= True')
                                                fns.write("\n\t\t"+" "*4+'for ix in range(0,len(nameList),1):')
                                                fns.write("\n\t\t\t"+" "*4+'if \"'+args[arg_index]+'\" == nameList[ix]:')
                                                fns.write("\n\t\t\t\t"+" "*4+'dataList[ix] = int(dataList[ix])')
                                                fns.write("\n\t\t\t\t"+" "*4+'acl_ptr_'+str(arg_index)+'.'+ub[attr_acl_proprty['attrib_type'][0]]+'= bool(dataList[ix])')
	                                    else:
                                                fns.write("\n\t\t"+" "*4+'acl_ptr_'+str(arg_index)+' = new_sai_attribute_value_tp()')
                                                fns.write("\n\t\t"+" "*4+'for ix in range(0,len(nameList),1):')
                                                fns.write("\n\t\t\t"+" "*4+'if \"'+args[arg_index]+'\" == nameList[ix]:')
                                                fns.write("\n\t\t\t\t"+" "*4+'dataList[ix] = int(dataList[ix])')
                                                fns.write("\n\t\t\t\t"+" "*4+'acl_ptr_'+str(arg_index)+'.'+ub[attr_acl_proprty['attrib_type'][0]]+'= dataList[ix]')
                                            fn_call = fn_call+'acl_ptr_'+str(arg_index)+'.'+ub[attr_acl_proprty['attrib_type'][0]]+','
                                            delete_string = delete_string + "\n\t\t"+" "*4+"delete_sai_attribute_value_tp(acl_ptr_"+str(arg_index)+")"
                                        fns.write("\n")
                                else:
                                    fn_call = fn_call+'int(args['+str(arg_index)+']),'
		    	

		    else:
                        if case == "default" and eachobjct == "policer" and arg_index == 3:
			    fn_call = fn_call+'eval(color_source),'
                        else:
			    fn_call = fn_call+'eval(args['+str(arg_index)+']),'
		    arg_index= arg_index+1
		    list_index= list_index+1

		if eachfile == 'saifdb':
		    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'(sai_fdb_entry_t_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\t"+" "*4+"delete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)"

                elif eachfile == 'sairoute':
                    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'(int(args[0]), int(args[1]), int(args[2]), ip_addr_ptr, ip_mask_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\t"+" "*4+"delete_shell_ip6p(ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\t"+" "*4+"delete_shell_ip6p(ip_mask_ptr)")

		elif eachfile == 'saineighbor':
                    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'(int(args[0]), int(args[1]), ip_addr_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\t"+" "*4+"delete_sai_ip_address_tp(ip_addr_ptr)"

                elif eachfile == 'saimcastfdb':
                    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'(sai_mcast_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\t"+" "*4+"delete_sai_mcast_fdb_entry_tp(sai_mcast_fdb_entry_ptr)"

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, '+fn_call
                    delete_string = delete_string+"\n\t\t"+" "*4+"delete_sai_ip_address_tp(src_ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\t"+" "*4+"delete_sai_ip_address_tp(dst_ip_addr_ptr)")

                else:
                    fn_call = "\n\t\t"+" "*4+'ret = '+ create['name'].replace('_fn','')+'_'+case+'('+fn_call
	    
		fn_call = fn_call[:-1]
		fns.write(fn_call+')')
                fns.write(delete_string)
                fns.write("\n\t\t"+" "*4+'xpShellGlobals.cmdRetVal = long(ret)')
                delete_string = ''
		fn_call = ''



def gen_set_func():

    filelist = db

    for eachfile in filelist.keys():

        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py',"a+") #Open the existing py file in append mode

	objctlist = filelist[eachfile]

	for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            attr_list = objctmodules['attr_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0

	#TODO Handling list types(array types), mac , ip6 

            if 'SET' in fn_list.keys():
    	        create = fn_list['SET']
	
		string =''
		delete_string =''
		list_flag = 0
		spcl_flag = 0
		index = 0
					
		fns.write("\n"+"    #/*********************************************************/")
		fns.write("\n"+"    # command for sai_set_"+eachobjct+"_attribute"               )
		fns.write("\n"+"    #/*********************************************************/")

		fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
		num_of_param = create['numparam']
		
		#There are few objects which does not have an Object-id associated with them like Fdb, Route, Neighbor
		#All these objects are identified by their entries in the hardware like fdb_entry, route_entry, neighbor_entry, etc.. 
		#so these ogjects need to be handled uniquely
			
		if eachfile == 'saifdb':#FdbEntry does not have an Object_id so to set an attribute user has to enter the fdb-entry so expansion of struct fdb-entry
		      string = string + ' switch_id, mac_address, bv_id,'
		      index = index+3
		
		elif eachfile == 'sairoute':#Expansion of route_entry is required as no object-id for this
		      string = string + 'switch_id, vr_id, ip_addr_family, ip_addr , ip_mask,'
		      index = index+5

		elif eachfile == 'saineighbor':#Expansion of route_entry is required as no object-id for this
                      string = string + 'switch_id, rif_id, ip_addr_family, ip_addr,'
                      index = index+4

                elif eachfile == 'saimcastfdb':#McastFdbEntry does not have an Object_id so to set an attribute user has to enter the mcast-fdb-entry so expansion of struct mcast-fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      index = index+3

                elif eachfile == 'sail2mc':#L2McEntry does not have an Object_id so to set an attribute user has to enter the l2mc-entry so expansion of struct l2mc-entry
                      string = string+' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip,'
                      index = index+6

                elif eachfile == 'saiipmc':#IPMcEntry does not have an Object_id so to set an attribute user has to enter the ipmc-entry so expansion of struct ipmc-entry
                      string = string+' switch_id, vr_id, type, ipaddr_family, src_ip, dst_ip,'
                      index = index+6

                elif eachfile == 'saiqosmap':
                      string = string+'qos_map_id, attr_id, qos_map_type, attr_value'
                      index = 1

		else:#If the file is none of the above find the primary arguments from the data base
		    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][2]+','
			    index = index+1

		if eachfile != 'saiqosmap':
			string = string + ' attr_id, attr_value'
		string = string.replace('*','')
		fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")
		
		#This part of code is takem from shellCmdsgen.py to parse the recieved arguments
		fns.write('\n\n\tvarStr = '+"'"+string+"'")
		fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
		args = string.replace(' ','').split(',')
		fns.write('\n\toptArg = '+'[ ')
		strng =''
		for p in args:
	            strng = strng +"'"+ p+"'"+','
		fns.write(strng[:-1]+' ]')

		fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
		if eachfile == 'saiqosmap':
			fns.write("\n\targsList = re.split(' ',arg)")
		else:
			fns.write("\n\n\targsList = re.split(',| ',arg)")
		fns.write("\n\targs = argsList")

		fns.write("\n\tif (totNumArgs > 0 and argsList[0] == ''):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
		fns.write("\n\telse:")
		
		#Need to parse the input arguments  differently for fdb, route, neighbor etc ..
		if eachfile == 'saifdb':
                    fns.write("\n\n\t\tsai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()")
		    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])")
		    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
		    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
		    fns.write("\n\t\t"+text)            	    
         	    fns.write("\n\t\tlistLen = len(postList)")
	            fns.write("\n\t\tfor ix in range(0, listLen, 1):")
	            fns.write(" \n\t\t\tsai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
		    fns.write("\n\t\targs[2]=int(args[2])")
		    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]")

		elif eachfile == 'sairoute':
                    fns.write("\n\t\tip_addr_ptr = new_shell_ip6p()")
		    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr[ix] = int(postList[ix], base)")

		    fns.write("\n\t\tip_mask_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_mask_ptr.addr[ix] = int(postList[ix], base)")

		elif eachfile == 'saineighbor':
		    fns.write("\n\t\tip_addr_ptr = new_sai_ip_address_tp()")
		    fns.write("\n\t\tip_addr_ptr.addr_family = eval(args[2])")
		    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")

                elif eachfile == 'saimcastfdb':
                    fns.write("\n\n\t\tsai_mcast_fdb_entry_ptr = new_sai_mcast_fdb_entry_tp()")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsai_mcast_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\targs[2]=int(args[2])")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.bv_id = args[2]")

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fns.write("\n\t\tsrc_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tsrc_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsrc_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")

                    fns.write("\n\t\tdst_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tdst_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[5] = args[5].replace(".",":").replace(",",":")')
                    text = 'postList = args[5].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tdst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
	
		else:
		    fns.write("\n\t\targs[0] = int(args[0])")	

		fns.write("\n\t\targs["+str(index)+"]=eval(args["+str(index)+"])")		
		fns.write("\n\n\t\tattr_value_ptr = new_sai_attribute_value_tp()")
                fns.write("\n\t\tipaddr_ptr = new_sai_ip_address_tp()")		
		fns.write("\n\t\tipaddrflag = 0")
		fns.write("\n\t\tlistLen = 0")
                if (eachobjct == "acl_entry"):
                    fns.write("\n\t\tacl_field_ptr = new_shell_acl_field_tp()")
                    fns.write("\n\t\tacl_action_ptr = new_shell_acl_action_tp()")
                    fns.write("\n\t\taclfieldflag = 0")
                    fns.write("\n\t\taclactionflag = 0")

		check_flag = 0 #Flag for maintaining branching of if and elseif in C code

		for attr_id in attr_list.keys():
		        attr_proprty = attr_list[attr_id]
		        if check_flag == 0:			    
			    fns.write("\n\t\tif args["+str(index)+"] == eval("+"'"+attr_id+"'"+'):')
			    check_flag = 1
		        else:
			    fns.write("\n\t\telif args["+str(index)+"] == eval("+"'"+attr_id+"'"+'):')
                        if attr_proprty['attrib_type'][0] in ub.keys():
				
			    #Copying the values into the union sai_attribute_value should be properly taken care as the union have differnt datatypes in it. 
			    
			    #MAC data type
			    if ub[attr_proprty['attrib_type'][0]] == "mac": 
                                fns.write("\n\t\t\t"+'args['+str(index+1)+'] = args['+str(index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t\t"+text)
                                fns.write("\n\t\t\tlistLen = len(postList)")
                                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                                fns.write(" \n\t\t\t\tattr_value_ptr.mac[ix] = int(postList[ix], 16)")

			    #IP6 data type
                            elif ub[attr_proprty['attrib_type'][0]] == "ip6":
                                fns.write("\n\t\t\t"+'args['+str(index+1)+'] = args['+str(index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'                     
                                fns.write("\n\t\t\t"+text)
                                fns.write("\n\t\t\tlistLen = len(postList)")
                                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                                fns.write(" \n\t\t\t\tattr_value_ptr.ip6[ix] = int(postList[ix])")

                            elif ub[attr_proprty['attrib_type'][0]] == "ip4":
                                fns.write("\n\t\t\t"+'args['+str(index+1)+'] = args['+str(index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'                     
                                fns.write("\n\t\t\t"+text)
                                fns.write("\n\t\t\tpostList = postList[::-1]")
                                fns.write("\n\t\t\tlistLen = len(postList)")
				fns.write("\n\t\t\tattr_value_ptr.ip4 = 0")
                                fns.write("\n\t\t\tfor ix in range(listLen):")
                                fns.write(" \n\t\t\t\tattr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])")

                            elif 'qos_map' in attr_proprty['attrib_type'][0]:
                                fns.write("""
			postList = []
			count = 0
			if args[3] != "[]":
				postList = args[3].strip(']').strip('[').split(":")
				listLen = len(postList)
			map_to_value_list_ptr = new_sai_qos_map_list_tp()
			map_to_value_list_ptr.count = listLen
			list_ptr_arr = new_arr_sai_qos_map(listLen)
			if ("SAI_QOS_MAP_TYPE" and "TO") not in args[2]:
				print " %s is Invalid argument" % (args[2])
				return
			nameList = args[2]
			nameList = nameList.replace("SAI_QOS_MAP_TYPE_","").replace("_","").lower().split("to")
			nameDict = {'queue':'queue_index', 'pfcpriority':'prio', 'prioritygroup':'pg'}
			for ix in range(0,len(nameList)):
				nameList[ix] = nameList[ix].split('and')
			for ix in range(0,len(postList)):
				postList[ix] = postList[ix].split(',')
			for ix in range(0,listLen):
				if(len(nameList[0] + nameList[1]) != len(postList[ix])):
					print " %s is Invalid argument" % (args[2])
					return
				list_ptr = arr_sai_qos_map_getitem(list_ptr_arr, ix)
				for ix1 in range(0,len(postList[ix]),1):
					if ix1 < len(nameList[0]):
						for key,value in nameDict.items():
							if nameList[0][ix1] == key:
								nameList[0][ix1] = value
						exec("list_ptr.key." + nameList[0][ix1] + " = "+postList[ix][ix1])
						count += 1
					else:
						for key,value in nameDict.items():
							if nameList[1][ix1 - count] == key:
								nameList[1][ix1 - count] = value
						exec("list_ptr.value." + nameList[1][ix1 - count] + " = "+postList[ix][ix1])
				count = 0
				arr_sai_qos_map_setitem(list_ptr_arr, ix, list_ptr)
			map_to_value_list_ptr.list = list_ptr_arr
			attr_value_ptr.qosmap = map_to_value_list_ptr""")
                                delete_string = """
			delete_sai_qos_map_list_tp(map_to_value_list_ptr)
			delete_arr_sai_qos_map(list_ptr_arr)"""

                            elif '_list_t' in attr_proprty['attrib_type'][0]:
                                fns.write("\n\t\t\t"+'if args['+str(index+1)+'] != "[]":')
                                fns.write("\n\t\t\t\t"+'args['+str(index+1)+'] = args['+str(index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t\t\t"+text)
                                fns.write("\n\t\t\t\tlistLen = len(postList)")
				fns.write("\n\t\t\tattr_value_ptr."+ub[attr_proprty['attrib_type'][0]] +'.count = listLen')
				fns.write("\n\t\t\tlist_ptr = new_arr_"+attr_proprty['attrib_type'][0].replace('_list_t','')+'(listLen)')
				delete_string = "\n\t\t\tdelete_arr_"+attr_proprty['attrib_type'][0].replace('_list_t','')+'(list_ptr)'
				fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
				fns.write("\n\t\t\t\tarr_"+attr_proprty['attrib_type'][0].replace('_list_t','')+'_setitem'+"(list_ptr, ix, int(postList[ix]))")
				fns.write("\n\t\t\tattr_value_ptr."+ub[attr_proprty['attrib_type'][0]] +'.list = list_ptr')				
			    elif ub[attr_proprty['attrib_type'][0]] == "booldata":
				fns.write("\n\t\t\t"+'args['+str(index+1)+']'+'= int(args['+str(index+1)+'])')
				fns.write("\n\t\t\t"+'attr_value_ptr.u8'+'= args['+str(index+1)+']')

			    elif ub[attr_proprty['attrib_type'][0]] == "ipaddr":
                                fns.write("\n\t\t\tipaddrflag = 1")
                                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t\t"+text)
                                fns.write("\n\t\t\tlistLen = len(postList)")
                                fns.write("\n\t\t\tipaddr_ptr.addr_family = eval(args["+str(index+1)+'])')
                                fns.write("\n\t\t\tbase = 10")
                                fns.write("\n\t\t\tif listLen > 4:")
                                fns.write("\n\t\t\t\tbase = 16")
                                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t\tipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)")

			    elif ub[attr_proprty['attrib_type'][0]] == "chardata":
				fns.write("\n\t\t\t"+'args['+str(index+1)+'] = args['+str(index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
				fns.write("\n\t\t\t"+text)
				fns.write("\n\t\t\tlistLen = len(postList[0])")
				fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
				fns.write("\n\t\t\t\tattr_value_ptr.chardata[ix] = postList[0][ix]")

                            elif ub[attr_proprty['attrib_type'][0]] == "aclfield":
                                fns.write("\n\t\t\taclfieldflag = 1 ")
                                delete_string = delete_string + gen_acl_set_api_body(fns, ub, attr_proprty, index, 'aclfield')

                            elif ub[attr_proprty['attrib_type'][0]] == "aclaction":
                                fns.write("\n\t\t\taclactionflag = 1 ")
                                delete_string = delete_string + gen_acl_set_api_body(fns, ub, attr_proprty, index, 'aclaction')
		
                            else:
				fns.write("\n\t\t\t"+'args['+str(index+1)+']'+'= int(args['+str(index+1)+'])')
                                fns.write("\n\t\t\t"+'attr_value_ptr.'+ub[attr_proprty['attrib_type'][0]]+'= args['+str(index+1)+']')
                        else:
                            fns.write("\n\t\t\t"+'attr_value_ptr.s32'+'= eval(args['+str(index+1)+'])')
	
	
			if eachfile == 'saifdb':
			    fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_fdb_entry_t_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)')
			    fns.write("\n\t\t\tdelete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)")
		    	    
    			elif eachfile == 'sairoute':
		    	    fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), int(args[2]), ip_addr_ptr, ip_mask_ptr, int(args[5]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)')
		    	    fns.write("\n\t\t\tdelete_shell_ip6p(ip_addr_ptr)")
			    fns.write("\n\t\t\tdelete_shell_ip6p(ip_mask_ptr)")
			
			elif eachfile == 'saineighbor':
			    fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), ip_addr_ptr, int(args[4]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)')
                            fns.write("\n\t\t\tdelete_sai_ip_address_tp(ip_addr_ptr)")

                        elif eachobjct == 'acl_entry':
                            fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(args[0], args[1], attr_value_ptr,int(aclfieldflag),acl_field_ptr, int(aclactionflag), acl_action_ptr)')

                        elif eachfile == 'saimcastfdb':
                            fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_mcast_fdb_entry_ptr, int(args[3]), attr_value_ptr, int(ipaddrflag), ipaddr_ptr)')
                            fns.write("\n\t\t\tdelete_sai_mcast_fdb_entry_tp(sai_mcast_fdb_entry_ptr)")

                        elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                            fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr, args[6], attr_value_ptr, int(ipaddrflag), ipaddr_ptr)')
                            fns.write("\n\t\t\tdelete_sai_ip_address_tp(src_ip_addr_ptr)")
                            fns.write("\n\t\t\tdelete_sai_ip_address_tp(dst_ip_addr_ptr)")


			else:
			    fns.write("\n\t\t\t"+'ret = '+ create['name'].replace('_fn','')+'(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)')
                            
			fns.write(delete_string)
			delete_string = ''
		fns.write("\n\t\telse:\n\t\t\tprint("+'"Invalid attr_id "+str(args['+str(index)+'])+" recieved.")')
		fns.write("\n\t\tdelete_sai_attribute_value_tp(attr_value_ptr)")
                fns.write("\n\t\tdelete_sai_ip_address_tp(ipaddr_ptr)")
                if eachobjct == 'acl_entry':
                    fns.write("\n\t\tdelete_shell_acl_field_tp(acl_field_ptr)")
                    fns.write("\n\t\tdelete_shell_acl_action_tp(acl_action_ptr)")
        fns.close             


def gen_get_func():

    filelist = db

    for eachfile in filelist.keys():
        #print eachfile
        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py','a+') #Open the existing py file in append mode

        objctlist = filelist[eachfile]

        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            attr_list = objctmodules['attr_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0

            #TODO Handling list types(array types), mac , ip6 

            if 'GET' in fn_list.keys():
		create = fn_list['GET']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                num_of_args = 0
                index = 0

                fns.write("\n\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_get_"+eachobjct+'_attribute'                 )
                fns.write("\n"+"    #/*********************************************************/")

                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
                num_of_param = create['numparam']

                #There are few objects which does not have an Object-id associated with them like Fdb, Route, Neighbor
                #All these objects are identified by their entries in the hardware like fdb_entry, route_entry, neighbor_entry, etc.. 
                #so these ogjects need to be handled uniquely

                list_size = 0
                fn_call = ''
                if eachfile == 'saifdb':#FdbEntry does not have an Object_id so to set an attribute user has to enter the fdb-entry so expansion of struct fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sairoute':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, vr_id, ip_addr_family, ip_addr , ip_mask,'
                      num_of_args = num_of_args+5

                elif eachfile == 'saineighbor':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, rif_id, ip_addr_family, ip_addr,'
                      num_of_args = num_of_args+4

                elif eachfile == 'saimcastfdb':#McastFdbEntry does not have an Object_id so to set an attribute user has to enter the mcast-fdb-entry so expansion of struct mcast-fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sail2mc':#L2McEntry does not have an Object_id so to set an attribute user has to enter the l2mc-entry so expansion of struct l2mc-entry
                      string = string+' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6

                elif eachfile == 'saiipmc':#IPMcEntry does not have an Object_id so to set an attribute user has to enter the ipmc-entry so expansion of struct ipmc-entry
                      string = string+' switch_id, vr_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6

                else:#If the file is none of the above find the primary arguments from the data base
                    for x in range(num_of_param):
			param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+' '+param_list[x][2]+','
                            attr_type_list.append(param_list[x][1])
                            attr_name_list.append(param_list[x][2])
                            list_size = list_size+1
                            num_of_args = num_of_args+1
	    
		string = string + ' attr_id, count_if_attribute_is_list'
                string = string.replace('*','')
                fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")

		#This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\tvarStr = '+"'"+string+"'")
                fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\toptArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')

                fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
                fns.write("\n\n\targsList = re.split(',| ',arg)")
                fns.write("\n\targs = argsList")

                fns.write("\n\tif (totNumArgs > 0 and argsList[0] == ''):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\telse:")

                #Need to parse the input arguments  differently for fdb, route, neighbor etc ..
                if eachfile == 'saifdb':
                    fns.write("\n\n\t\tsai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()")
                    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\targs[2]=int(args[2])")
                    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]")
                    arg_index = 3

		elif eachfile == 'sairoute':
                    fns.write("\n\t\tip_addr_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr[ix] = int(postList[ix])")

                    fns.write("\n\t\tip_mask_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_mask_ptr.addr[ix] = int(postList[ix])")
                    arg_index = 5

                elif eachfile == 'saineighbor':
                    fns.write("\n\t\tip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tip_addr_ptr.addr_family = eval(args[2])")
                    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
                    arg_index = 4

                elif eachfile == 'saimcastfdb':
                    fns.write("\n\n\t\tsai_mcast_fdb_entry_ptr = new_sai_mcast_fdb_entry_tp()")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsai_mcast_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\targs[2]=int(args[2])")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.bv_id = args[2]")
                    arg_index = 3

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fns.write("\n\t\tsrc_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tsrc_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsrc_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")

                    fns.write("\n\t\tdst_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tdst_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[5] = args[5].replace(".",":").replace(",",":")')
                    text = 'postList = args[5].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tdst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
                    arg_index = 6

                else:
                    arg_index = 0

		list_index = 0
                while arg_index < num_of_args and list_index < list_size:
                    #print list_index
                    if attr_type_list[list_index] in ub.keys():

                            #MAC data type
                            if  ub[attr_type_list[list_index]] == "mac":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_macp()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+'_ptr.mac[ix] =  int(postList[ix], 16)')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'
                            #IP6 data type
                            elif ub[attr_type_list[list_index]] == "ip6":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_ip6p()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+'_ptr.ip6[ix] = int(postList[ix])')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            elif ub[attr_type_list[list_index]] == "ip4":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tpostList = postList[::-1]")
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\tx =0")
                                fns.write("\n\t\tfor ix in range(listLen):")
				fns.write(" \n\t\t\tx = (x*256)+int(postList[ix])")
                                fn_call = fn_call+' int(x),'

                            elif '_list_t' in attr_type_list[list_index]:
                                fns.write("\n\t\t"+'args['+str(arg_index+1)+'] = args['+str(arg_index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr = new_'+attr_type_list[list_index+1]+'p()')
                                delete_string = delete_string+"\n\t\t"+'delete_'+attr_type_list[list_index+1]+'p('+attr_name_list[list_index+1]+'_ptr)'
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr.count = int(args['+str(arg_index)+'])')
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_list_ptr = new_arr_'+attr_type_list[list_index+1].replace('_list_t','')+'(int('+'args['+str(arg_index)+']'+'))')
                                delete_string = delete_string+"\n\t\tdelete_arr_"+attr_type_list[list_index+1].replace('_list_t','')+'('+attr_name_list[list_index]+'_list_ptr)'
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\tarr_"+attr_type_list[list_index+1].replace('_list_t','')+'_setitem'+'('+attr_name_list[list_index]+"_list_ptr, ix, int(postList[ix]))")
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr.list = '+attr_name_list[list_index]+'_list_ptr')
                                fn_call = fn_call+attr_name_list[list_index+1]+'_ptr,'
                                arg_index= arg_index+1
                                list_index= list_index+1

                            elif ub[attr_type_list[list_index]] == "booldata":
                                fn_call = fn_call+'int(args['+str(arg_index)+']) ,'

                            elif ub[attr_type_list[list_index]] == "ipaddr":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index+1)+'] = args['+str(arg_index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr = new_sai_ip_address_tp()')
                                delete_string = delete_string+"\n\t\t"+" "*4+'delete_sai_ip_address_tp('+attr_name_list[list_index+1]+'_ptr)'
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr_family = eval(args['+str(arg_index)+'])')
                                fns.write("\n\t\t" +" "*4+ "base = 10")
                                fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                                fns.write("\n\t\t\t" +" "*4+ "base = 16")

                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr.ip6[ix] = int(postList[ix], base)')
                                fn_call = fn_call+attr_name_list[list_index+1]+'_ptr,'
                                arg_index= arg_index+1
                                list_index= list_index+1

                            elif ub[attr_type_list[list_index]] == "chardata":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList[0])")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_stringp()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+"_ptr.name[ix] = postList[0][ix]")
                                delete_string = delete_string+"\n\t\tdelete_"+'shell_stringp('+attr_name_list[list_index]+"_ptr)"
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            else:
                                fn_call = fn_call+'int(args['+str(arg_index)+']),'


                    else:
                        fn_call = fn_call+'eval(args['+str(arg_index)+']),'
		    
                    arg_index= arg_index+1
                    list_index= list_index+1

		fn_call = fn_call + ' eval(args['+str(arg_index)+']),'+' int(args['+str(arg_index+1)+']),None,'

                if eachfile == 'saifdb':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_fdb_entry_t_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)"

                elif eachfile == 'sairoute':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), int(args[2]), ip_addr_ptr, ip_mask_ptr,'+fn_call
		    delete_string = delete_string+"\n\t\tdelete_shell_ip6p(ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\tdelete_shell_ip6p(ip_mask_ptr)")

                elif eachfile == 'saineighbor':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), ip_addr_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_ip_address_tp(ip_addr_ptr)"

                elif eachfile == 'saimcastfdb':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_mcast_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_mcast_fdb_entry_tp(sai_mcast_fdb_entry_ptr)"

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_ip_address_tp(src_ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\tdelete_sai_ip_address_tp(dst_ip_addr_ptr)")

                else:
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'('+fn_call

		
                fn_call = fn_call[:-1]
                fns.write(fn_call+')')
                fns.write(delete_string)
                fns.write("\n\t\t"+'xpShellGlobals.cmdRetVal = long(ret)')
                delete_string = ''
                fn_call = ''


def gen_remove_func():

    filelist = db

    for eachfile in filelist.keys():
        #print eachfile
        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
	fns = open('saiShellPyWrappers/'+file_name+'.py','a+') #Open the existing py file in append mode

        objctlist = filelist[eachfile]

        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            attr_list = objctmodules['attr_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0

            #TODO Handling list types(array types), mac , ip6 

            if 'REMOVE' in fn_list.keys():
                create = fn_list['REMOVE']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                num_of_args = 0
                index = 0

                fns.write("\n\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_remove_"+eachobjct                          )
                fns.write("\n"+"    #/*********************************************************/")

                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
		num_of_param = create['numparam']

                #There are few objects which does not have an Object-id associated with them like Fdb, Route, Neighbor
                #All these objects are identified by their entries in the hardware like fdb_entry, route_entry, neighbor_entry, etc.. 
                #so these ogjects need to be handled uniquely

                list_size = 0
                fn_call = ''
                if eachfile == 'saifdb':#FdbEntry does not have an Object_id so to set an attribute user has to enter the fdb-entry so expansion of struct fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sairoute':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, vr_id, ip_addr_family, ip_addr , ip_mask,'
                      num_of_args = num_of_args+5

                elif eachfile == 'saineighbor':#Expansion of route_entry is required as no object-id for this
                      string = string+ 'switch_id, rif_id, ip_addr_family, ip_addr,'
                      num_of_args = num_of_args+4

                elif eachfile == 'saimcastfdb':#McastFdbEntry does not have an Object_id so to set an attribute user has to enter the mcast-fdb-entry so expansion of struct mcast-fdb-entry
                      string = string+' switch_id, mac_address, bv_id,'
                      num_of_args = num_of_args+3

                elif eachfile == 'sail2mc':#L2McEntry does not have an Object_id so to set an attribute user has to enter the l2mc-entry so expansion of struct l2mc-entry
                      string = string+' switch_id, bv_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6

                elif eachfile == 'saiipmc':#IPMcEntry does not have an Object_id so to set an attribute user has to enter the ipmc-entry so expansion of struct ipmc-entry
                      string = string+' switch_id, vr_id, type, ipaddr_family, src_ip, dst_ip,'
                      num_of_args = num_of_args+6


                else:#If the file is none of the above find the primary arguments from the data base
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+' '+param_list[x][2]+','
                            attr_type_list.append(param_list[x][1])
                            attr_name_list.append(param_list[x][2])
                            list_size = list_size+1
                            num_of_args = num_of_args+1

                string = string.replace('*','')
		string = string[:-1]
                fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")

		#This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\tvarStr = '+"'"+string+"'")
                fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\toptArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')

                fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
                fns.write("\n\n\targsList = re.split(',| ',arg)")
                fns.write("\n\targs = argsList")

                fns.write("\n\tif (totNumArgs > 0 and argsList[0] == ''):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\telse:")

                #Need to parse the input arguments  differently for fdb, route, neighbor etc ..
                if eachfile == 'saifdb':
                    fns.write("\n\n\t\tsai_fdb_entry_t_fdb_entry_ptr = new_sai_fdb_entry_tp()")
                    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsai_fdb_entry_t_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\targs[2]=int(args[2])")
                    fns.write("\n\t\tsai_fdb_entry_t_fdb_entry_ptr.bv_id = args[2]")
                    arg_index = 3

                elif eachfile == 'sairoute':
                    fns.write("\n\t\tip_addr_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr[ix] = int(postList[ix], base)")

                    fns.write("\n\t\tip_mask_ptr = new_shell_ip6p()")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_mask_ptr.addr[ix] = int(postList[ix], base)")
		    arg_index = 5

                elif eachfile == 'saineighbor':
                    fns.write("\n\t\tip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tip_addr_ptr.addr_family = eval(args[2])")
                    fns.write("\n\t\t"+'args[3] = args[3].replace(".",":").replace(",",":")')
                    text = 'postList = args[3].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
                    arg_index = 4

                elif eachfile == 'saimcastfdb':
                    fns.write("\n\n\t\tsai_mcast_fdb_entry_ptr = new_sai_mcast_fdb_entry_tp()")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.switch_id = int(args[0])")
                    fns.write("\n\t\t"+'args[1] = args[1].replace(".",":").replace(",",":")')
                    text = 'postList = args[1].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsai_mcast_fdb_entry_ptr.mac_address[ix] = int(postList[ix], 16)")
                    fns.write("\n\t\targs[2]=int(args[2])")
                    fns.write("\n\t\tsai_mcast_fdb_entry_ptr.bv_id = args[2]")
                    arg_index = 3

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fns.write("\n\t\tsrc_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tsrc_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[4] = args[4].replace(".",":").replace(",",":")')
                    text = 'postList = args[4].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tsrc_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")

                    fns.write("\n\t\tdst_ip_addr_ptr = new_sai_ip_address_tp()")
                    fns.write("\n\t\tdst_ip_addr_ptr.addr_family = eval(args[3])")
                    fns.write("\n\t\t"+'args[5] = args[5].replace(".",":").replace(",",":")')
                    text = 'postList = args[5].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                    fns.write("\n\t\t"+text)
                    fns.write("\n\t\tlistLen = len(postList)")
                    fns.write("\n\t\tbase = 10")
                    fns.write("\n\t\tif listLen > 4:")
                    fns.write("\n\t\t\tbase = 16")
                    fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                    fns.write(" \n\t\t\tdst_ip_addr_ptr.addr.ip6[ix] = int(postList[ix], base)")
                    arg_index = 6

                else:
                    arg_index = 0

                list_index = 0
                while arg_index < num_of_args and list_index < list_size:
                    #print list_index
                    if attr_type_list[list_index] in ub.keys():

                            #MAC data type
                            if  ub[attr_type_list[list_index]] == "mac":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_macp()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+'_ptr.mac[ix] =  int(postList[ix], 16)')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'
                            #IP6 data type
			    elif ub[attr_type_list[list_index]] == "ip6":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_ip6p()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+'_ptr.ip6[ix] = int(postList[ix])')
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            elif ub[attr_type_list[list_index]] == "ip4":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tpostList = postList[::-1]")
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\tx =0")
                                fns.write("\n\t\tfor ix in range(listLen):")
                                fns.write(" \n\t\t\tx = (x*256)+int(postList[ix])")
                                fn_call = fn_call+' int(x),'

                            elif '_list_t' in attr_type_list[list_index]:
                                fns.write("\n\t\t"+'args['+str(arg_index+1)+'] = args['+str(arg_index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList)")
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr = new_'+attr_type_list[list_index+1]+'p()')
                                delete_string = delete_string+"\n\t\t"+'delete_'+attr_type_list[list_index+1]+'p('+attr_name_list[list_index+1]+'_ptr)'
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr.count = int(args['+str(arg_index)+'])')
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+"_list_ptr = new_arr_"+attr_type_list[list_index+1].replace('_list_t','')+'(int('+'args['+str(arg_index)+']'+'))')
                                delete_string = delete_string+"\n\t\tdelete_arr_"+attr_type_list[list_index+1].replace('_list_t','')+'('+attr_name_list[list_index+1]+'_list_ptr)'
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\tarr_"+attr_type_list[list_index+1].replace('_list_t','')+'_setitem'+'('+attr_name_list[list_index+1]+"_list_ptr, ix, int(postList[ix]))")
                                fns.write("\n\t\t"+attr_name_list[list_index+1]+'_ptr.list = '+attr_name_list[list_index+1]+'_list_ptr')
                                fn_call = fn_call+attr_name_list[list_index+1]+'_ptr,'
                                arg_index= arg_index+1
                                list_index= list_index+1

			    elif ub[attr_type_list[list_index]] == "booldata":
                                fn_call = fn_call+'int(args['+str(arg_index)+']) ,'

                            elif ub[attr_type_list[list_index]] == "ipaddr":
                                fns.write("\n\t\t"+" "*4+'args['+str(arg_index+1)+'] = args['+str(arg_index+1)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index+1)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+" "*4+text)
                                fns.write("\n\t\t"+" "*4+"listLen = len(postList)")
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr = new_sai_ip_address_tp()')
                                delete_string = delete_string+"\n\t\t"+" "*4+'delete_sai_ip_address_tp('+attr_name_list[list_index+1]+'_ptr)'
                                fns.write("\n\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr_family = eval(args['+str(arg_index)+'])')
                                fns.write("\n\t\t" +" "*4+ "base = 10")
                                fns.write("\n\t\t" +" "*4+ "if listLen > 4:")
                                fns.write("\n\t\t\t" +" "*4+ "base = 16")
                                fns.write("\n\t\t"+" "*4+"for ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+" "*4+attr_name_list[list_index+1]+'_ptr.addr.ip6[ix] = int(postList[ix], base)')
                                fn_call = fn_call+attr_name_list[list_index+1]+'_ptr,'
                                arg_index= arg_index+1
                                list_index= list_index+1

                            elif ub[attr_type_list[list_index]] == "chardata":
                                fns.write("\n\t\t"+'args['+str(arg_index)+'] = args['+str(arg_index)+'].replace(".",":").replace(",",":")')
                                text = 'postList = args['+str(arg_index)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                                fns.write("\n\t\t"+text)
                                fns.write("\n\t\tlistLen = len(postList[0])")
                                fns.write("\n\t\t"+attr_name_list[list_index]+'_ptr = new_shell_stringp()')
                                fns.write("\n\t\tfor ix in range(0, listLen, 1):")
                                fns.write("\n\t\t\t"+attr_name_list[list_index]+"_ptr.name[ix] = postList[0][ix]")
                                delete_string = delete_string+"\n\t\tdelete_"+'shell_stringp('+attr_name_list[list_index]+"_ptr)"
                                fn_call = fn_call+attr_name_list[list_index]+'_ptr,'

                            else:
                                fn_call = fn_call+'int(args['+str(arg_index)+']),'


                    else:
                        fn_call = fn_call+'eval(args['+str(arg_index)+']),'

                    arg_index= arg_index+1
                    list_index= list_index+1

                if eachfile == 'saifdb':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_fdb_entry_t_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_fdb_entry_tp(sai_fdb_entry_t_fdb_entry_ptr)"

                elif eachfile == 'sairoute':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), int(args[2]), ip_addr_ptr, ip_mask_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_shell_ip6p(ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\tdelete_shell_ip6p(ip_mask_ptr)")

                elif eachfile == 'saineighbor':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), ip_addr_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_ip_address_tp(ip_addr_ptr)"

                elif eachfile == 'saimcastfdb':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(sai_mcast_fdb_entry_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_mcast_fdb_entry_tp(sai_mcast_fdb_entry_ptr)"

                elif eachfile == 'sail2mc' or eachfile == 'saiipmc':
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'(int(args[0]), int(args[1]), eval(args[2]), src_ip_addr_ptr, dst_ip_addr_ptr,'+fn_call
                    delete_string = delete_string+"\n\t\tdelete_sai_ip_address_tp(src_ip_addr_ptr)"
                    delete_string = delete_string+("\n\t\tdelete_sai_ip_address_tp(dst_ip_addr_ptr)")

                else:
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','')+'('+fn_call


                fn_call = fn_call[:-1]
                fns.write(fn_call+')')
                fns.write(delete_string)
                fns.write("\n\t\t"+'xpShellGlobals.cmdRetVal = long(ret)')
                delete_string = ''
                fn_call = ''

def gen_acl_set_api_body(fns, ub, attr_proprty, index, maintype):

    delete_string =''
    if maintype == 'aclfield':
        if attr_proprty['attrib_type'][1] in ub.keys():        

            if ub[attr_proprty['attrib_type'][1]] == "mac":
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_field_ptr.mask.mac[ix] = int(postList[ix], 16)")
                fns.write("\n\t\t\t"+'args['+str(index+3)+'] = args['+str(index+3)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+3)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_field_ptr.data.mac[ix] = int(postList[ix], 16)")

            elif ub[attr_proprty['attrib_type'][1]] == "ip4":
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tpostList = postList[::-1]")
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_field_ptr.mask.ip4 = 0")
                fns.write("\n\t\t\tfor ix in range(listLen):")
                fns.write(" \n\t\t\t\tacl_field_ptr.mask.ip4 = (acl_field_ptr.mask.ip4*256)+int(postList[ix])")

                fns.write("\n\t\t\t"+'args['+str(index+3)+'] = args['+str(index+3)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+3)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tpostList = postList[::-1]")
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_field_ptr.data.ip4 = 0")
                fns.write("\n\t\t\tfor ix in range(listLen):")
                fns.write(" \n\t\t\t\tacl_field_ptr.data.ip4 = (acl_field_ptr.data.ip4*256)+int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == "ip6":
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_field_ptr.mask.ip6[ix] = int(postList[ix])")
                fns.write("\n\t\t\t"+'args['+str(index+3)+'] = args['+str(index+3)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+3)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_field_ptr.data.ip6[ix] = int(postList[ix],16)")

            elif ub[attr_proprty['attrib_type'][1]] == 'objlist':
                
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_field_ptr.data.objlist.count = listLen")
                fns.write("\n\t\t\tlist_ptr = new_arr_sai_object(listLen)")
                delete_string = "\n\t\t\tdelete_arr_sai_object(list_ptr)"
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\tarr_sai_object_setitem(list_ptr, ix, int(postList[ix]))")
                fns.write("\n\t\t\tacl_field_ptr.data.objlist.list = list_ptr")

            elif ub[attr_proprty['attrib_type'][1]] == 'u8list':

                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_field_ptr.mask.u8list.count = listLen")
                fns.write("\n\t\t\tlist_ptr1 = new_arr_sai_u8(listLen)")
                delete_string = "\n\t\t\tdelete_arr_sai_u8(list_ptr1)"
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\tarr_sai_u8_setitem(list_ptr1, ix, int(postList[ix]))")
                fns.write("\n\t\t\tacl_field_ptr.mask.u8list.list = list_ptr1")

                fns.write("\n\t\t\t"+'args['+str(index+3)+'] = args['+str(index+3)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+3)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_field_ptr.data.u8list.count = listLen")
                fns.write("\n\t\t\tlist_ptr2 = new_arr_sai_u8(listLen)")
                delete_string = "\n\t\t\tdelete_arr_sai_u8(list_ptr2)"
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\tarr_sai_u8_setitem(list_ptr2, ix, int(postList[ix]))")
                fns.write("\n\t\t\tacl_field_ptr.data.u8list.list = list_ptr2")

            elif ub[attr_proprty['attrib_type'][1]] == 'booldata':
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\tacl_field_ptr.data.u8 = int(args["+str(index+2)+'])')

            elif ub[attr_proprty['attrib_type'][1]] == 'oid':
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\tacl_field_ptr.data.oid = int(args["+str(index+2)+'])')

            else:
                fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'acl_field_ptr.mask.'+ub[attr_proprty['attrib_type'][1]]+'= int(args['+str(index+2)+'])')
                fns.write("\n\t\t\t"+'acl_field_ptr.data.'+ub[attr_proprty['attrib_type'][1]]+'= int(args['+str(index+3)+'])')

        else:
            fns.write("\n\t\t\tacl_field_ptr.enable = bool(int(args["+str(index+1)+']))')
            fns.write("\n\t\t\tacl_field_ptr.mask.s32 = int(args["+str(index+2)+'])')
            fns.write("\n\t\t\tacl_field_ptr.data.s32 = int(args["+str(index+3)+'])')

    elif maintype == 'aclaction':
        if attr_proprty['attrib_type'][1] in ub.keys():        

            if ub[attr_proprty['attrib_type'][1]] == "mac":
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_action_ptr.parameter.mac[ix] = int(postList[ix], 16)")

            elif ub[attr_proprty['attrib_type'][1]] == "ip4":
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tpostList = postList[::-1]")
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_action_ptr.parameter.ip4 = 0")
                fns.write("\n\t\t\tfor ix in range(listLen):")
                fns.write(" \n\t\t\t\tacl_action_ptr.parameter.ip4 = (acl_action_ptr.parameter.ip4*256)+int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == "ip6":
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write(" \n\t\t\t\tacl_action_ptr.parameter.ip6[ix] = int(postList[ix],16)")

            elif ub[attr_proprty['attrib_type'][1]] == 'objlist':
                
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'args['+str(index+2)+'] = args['+str(index+2)+'].replace(".",":").replace(",",":")')
                text = 'postList = args['+str(index+2)+'].strip('+"'"+','+"'"+').strip('+'"'+"'"+'"'+').strip('+"'"+']'+"'"+')'+'.strip('+"'"+'['+"'"+').split('+"'"+':'+"'"+')'
                fns.write("\n\t\t\t"+text)
                fns.write("\n\t\t\tlistLen = len(postList)")
                fns.write("\n\t\t\tacl_action_ptr.parameter.objlist.count = listLen")
                fns.write("\n\t\t\tlist_ptr = new_arr_sai_object(listLen)")
                delete_string = "\n\t\t\tdelete_arr_sai_object(list_ptr)"
                fns.write("\n\t\t\tfor ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\tarr_sai_object_setitem(list_ptr, ix, int(postList[ix]))")
                fns.write("\n\t\t\tacl_action_ptr.parameter.objlist.list = list_ptr")

            elif ub[attr_proprty['attrib_type'][1]] == 'oid':
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\tacl_action_ptr.parameter.oid = int(args["+str(index+2)+'])')

            else:
                fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
                fns.write("\n\t\t\t"+'acl_action_ptr.parameter.'+ub[attr_proprty['attrib_type'][1]]+'= int(args['+str(index+2)+'])')

        else:
            fns.write("\n\t\t\tacl_action_ptr.enable = bool(int(args["+str(index+1)+']))')
            fns.write("\n\t\t\tacl_action_ptr.parameter.s32 = eval(args["+str(index+2)+'])')

    return delete_string

def gen_alter_create_func():
    
    '''Toggles the Create mode from Only MANDATORY_ON_CREATE to MANDATORY_ON_CREATE along with CREATE_AND_SET'''

    filelist = db

    for eachfile in filelist.keys():
	#print eachfile
        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py','a+') #Open the existing py file in append mode

        funcName = 'alter_create_mode'
        param = 'create_mode'
	isCreatePresentPerFile = 0

        objctlist = filelist[eachfile]


	#Checking if there is any create function in the file
        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]
            fn_list = objctmodules['fn_list']            

            #If there is a create function then only there will be the enable create mode toggel function
            if 'CREATE' in fn_list.keys():
                isCreatePresentPerFile = 1

        if isCreatePresentPerFile == 1:

            string =param

            fns.write("\n"+"    #/*********************************************************/")
            fns.write("\n"+"    # command for "+funcName)
            fns.write("\n"+"    #/*********************************************************/")

            fns.write("\n"+" "*4+"def do_"+funcName+'(self, arg):')
            fns.write("\n\n"+" "*8+"'''"+funcName+": Enter [ '"+param+"' ] '''")
            #fns.write("\n\n"+" "*8+"'''")
            #fns.write
                
            #This part of code is taken from shellCmdsgen.py to parse the recieved arguments
            fns.write('\n\n'+" "*8+'varStr = '+"'"+string+"'")
            fns.write('\n'+" "*8+'varList = '+"varStr.replace(' ', '').split(',')")
            args = string.replace(' ','').split(',')
            fns.write('\n'+" "*8+'optArg = '+'[ ')
            strng =''
            for p in args:
                strng = strng +"'"+ p+"'"+','
            fns.write(strng[:-1]+' ]')

            fns.write("\n"+" "*8+"args = ['0']*len(varList)"+"\n"+" "*8+"totNumArgs = len(varList)")
            fns.write("\n\n"+" "*8+"argsList = re.split(',| ',arg)")
            fns.write("\n"+" "*8+"args = argsList")

            fns.write("\n"+" "*8+"if (totNumArgs > 0 and argsList[0] == ''):"+"\n"+" "*12+"print('Invalid input, Enter [ %s ]' % varStr)")
            fns.write("\n"+" "*8+"else:")
		
            fns.write("\n"+" "*12+""+'xpShellGlobals.cmdRetVal = long(int(args[0]))')
            fns.write("\n"+" "*12+""+'saiShellGlobals.createMode = int(args[0])')
                
        fns.close()
                
                

#/*****************************************************************************************************/
# command to fns.write the header
#/*****************************************************************************************************/
def printCmdHeader(fns,f_name):
    fns.write('#!/usr/bin/env python')
    fns.write('\n#  %s' % (f_name))
    fns.write('\n#')
    fns.write('\n#*********************************************************************************/')
    fns.write('\n#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */')
    fns.write('\n#*  subject to the limited use license agreement by and between Marvell and you, */')
    fns.write('\n#*  your employer or other entity on behalf of whom you act. In the absence of   */')
    fns.write('\n#*  such license agreement the following file is subject to Marvell\'s standard   */')
    fns.write('\n#*  Limited Use License Agreement.                                               */')
    fns.write('\n#*********************************************************************************/')
    fns.write('\n#* @file %s' % (f_name))
    fns.write('\n#')
    fns.write('\n')
    fns.write('\nimport sys')
    fns.write('\nimport time')
    fns.write('\nimport os')
    fns.write('\nimport re')
    fns.write('\nimport readline')
    fns.write('\n')
    fns.write('\n#/**********************************************************************************/')
    fns.write('\n# import cmd2 package')
    fns.write('\n#/**********************************************************************************/')
    fns.write('\ndirname, filename = os.path.split(os.path.abspath(__file__))')
    fns.write('\nsys.path.append(dirname + "/../cli")')
    fns.write('\nsys.path.append(dirname + "/../../cli")')
    fns.write('\nfrom cmd2x import Cmd')
    fns.write('\nimport xpShellGlobals')
    fns.write('\nimport saiShellGlobals')
    fns.write('\nimport enumDict')
    fns.write('\n')
    fns.write('\n#/**********************************************************************************/')
    fns.write('\n# import everything from buildTarget')
    fns.write('\nfrom buildTarget import *')
    fns.write('\n')

def gen_acl_creat_api_body(fns, ub, attr_proprty, index, maintype, valList):

    delete_string =''
    if maintype == 'aclfield':
        if attr_proprty['attrib_type'][1] in ub.keys():

            if ub[attr_proprty['attrib_type'][1]] == "mac":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+'.enable = True')
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.mac[ix] = int(postList[ix], 16)")
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[1].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.mac[ix] = int(postList[ix], 16)")

            elif ub[attr_proprty['attrib_type'][1]] == "ip4":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+'.enable = True')
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = postList[::-1]')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.ip4 = 0")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(listLen):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.ip4 = (acl_ptr_"+str(index)+".mask.ip4*256)+int(postList[ix])")
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = valList[1].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = postList[::-1]')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.ip4 = 0")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(listLen):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.ip4 = (acl_ptr_"+str(index)+".data.ip4*256)+int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == "ip6":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+'.enable = True')
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.ip6[ix] = int(postList[ix])")
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[1].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.ip6[ix] = int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == 'objlist':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+'.enable = True')
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.objlist.count = listLen")
                fns.write("\n\t\t\t\t"+' '*4+"list_ptr = new_arr_sai_object(listLen)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.objlist.list = list_ptr")

            elif ub[attr_proprty['attrib_type'][1]] == 'u8list':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+'.enable = True')
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.u8list.count = listLen")
                fns.write("\n\t\t\t\t"+' '*4+"list_ptr1 = new_arr_sai_u8(listLen)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"arr_sai_u8_setitem(list_ptr1, ix, int(postList[ix]))")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.u8list.list = list_ptr1")
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[1].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.u8list.count = listLen")
                fns.write("\n\t\t\t\t"+' '*4+"list_ptr2 = new_arr_sai_u8(listLen)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"arr_sai_u8_setitem(list_ptr2, ix, int(postList[ix]))")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.u8list.list = list_ptr2")

            elif ub[attr_proprty['attrib_type'][1]] == 'booldata':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.u8 = int(valList[0])")

            elif ub[attr_proprty['attrib_type'][1]] == 'oid':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.oid = int(valList[0])")

            else:
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask."+ub[attr_proprty['attrib_type'][1]]+"= int(valList[0])")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data."+ub[attr_proprty['attrib_type'][1]]+"= int(valList[1])")

        else:
            fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
            fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".mask.s32 = int(valList[0])")
            fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".data.s32 = int(valList[1])")

    elif maintype == 'aclaction':
        if attr_proprty['attrib_type'][1] in ub.keys():

            if ub[attr_proprty['attrib_type'][1]] == "mac":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.mac[ix] = int(postList[ix], 16)")

            elif ub[attr_proprty['attrib_type'][1]] == "ip4":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = postList[::-1]')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.ip4 = 0")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(listLen):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.ip4 = (acl_ptr_"+str(index)+".parameter.ip4*256)+int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == "ip6":
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.ip6[ix] = int(postList[ix])")

            elif ub[attr_proprty['attrib_type'][1]] == 'objlist':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+""+'postList = valList[0].split(".")')
                fns.write("\n\t\t\t\t"+' '*4+"listLen = len(postList)")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.objlist.count = listLen")
                fns.write("\n\t\t\t\t"+' '*4+"list_ptr = new_arr_sai_object(listLen)")
                fns.write("\n\t\t\t\t"+' '*4+"for ix in range(0, listLen, 1):")
                fns.write("\n\t\t\t\t\t"+' '*4+"arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.objlist.list = list_ptr")

            elif ub[attr_proprty['attrib_type'][1]] == 'oid':
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.oid = int(valList[0])")

            else:
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
                fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter."+ub[attr_proprty['attrib_type'][1]]+"= int(valList[0])")

        else:
            fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".enable = True")
            fns.write("\n\t\t\t\t"+' '*4+"acl_ptr_"+str(index)+".parameter.s32 = int(valList[0])")

def gen_get_stats_func():

    filelist = db

    for eachfile in filelist.keys():

        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py',"a+") #Open the existing py file in append mode

        objctlist = filelist[eachfile]
        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0

            #TODO Handling list types(array types), mac , ip6 

            if 'GET_STATS' in fn_list.keys():
                if 'stats_attr_list' in objctmodules.keys():
                    stats_attr_list = objctmodules['stats_attr_list']
                create = fn_list['GET_STATS']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                index = 0
                list_size = 0
                num_of_args = 0
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_get_"+eachobjct+"_stats"               )
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
                num_of_param = create['numparam'] - 1
                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        if (x == (num_of_param - 1)):
                            string = string+' '+param_list[x][2]
                        else:
                            string = string+' '+param_list[x][2]+','
                        attr_type_list.append(param_list[x][1])
                        attr_name_list.append(param_list[x][2])
                        list_size = list_size+1
                        num_of_args = num_of_args+1
                string = string.replace('*','')
                fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")
                #This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\tvarStr = '+"'"+string+"'")
                fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\toptArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')
                fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
                fns.write("\n\n\targsList = re.split(',| ',arg)")
                fns.write("\n\targs = argsList")
                fns.write("\n\tif ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\telse:")
                fns.write("\n\t\tcounters_list = new_arrUint64(len(argsList) - 1)")
                command_id = '\n\t\tarr_stat_ptr = new_arr_' + 'sai_stat_id' + "_tp(len(argsList) - 1)"
                fns.write(command_id)
                fns.write("\n\t\targsLen = len(argsList)")
                fns.write("\n\t\tfor ix in range(0, argsLen - 1):")
                fns.write("\n\t\t\tarr_" + 'sai_stat_id' +"_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))");
                if eachobjct == "tam_snapshot":
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','') + "(int(args[0]),(argsLen - 1),arr_stat_ptr)"
                elif eachobjct == "tam_histogram":
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','') + "(int(args[0]),(argsLen - 1),counters_list)"
                else:
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','') + "(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)"
                fns.write(fn_call)
                fns.write("\n\t\tif(ret != 0):")
                fns.write("\n\t\t\tprint('Failled with RC: %d' % ret)")
                fns.write("\n\t\telse:")
                fns.write("\n\t\t\tresultStr = ''")
                fns.write("\n\t\t\tfor ix in range(0, argsLen - 1):")
                fns.write("\n\t\t\t\ttemp = arrUint64_getitem(counters_list, ix)")
                string = "\n\t\t\t\tresultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\\n'"
                fns.write(string)
                fns.write("\n\t\t\tprint('%s' % resultStr)")
                delete_comamnd_id = '\n\t\tdelete_arr_' + 'sai_stat_id' + '_tp(arr_stat_ptr)'
                fns.write(delete_comamnd_id)
                fns.write("\n\t\tdelete_arrUint64(counters_list)")

            if 'CLEAR_STATS' in fn_list.keys():
                if 'stats_attr_list' in objctmodules.keys():
                    stats_attr_list = objctmodules['stats_attr_list']
                create = fn_list['CLEAR_STATS']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                index = 0
                list_size = 0
                num_of_args = 0
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_clear_"+eachobjct+"_stats"               )
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
                num_of_param = create['numparam'] - 1
                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        if (x == (num_of_param - 1)):
                            string = string+' '+param_list[x][2]
                        else:
                            string = string+' '+param_list[x][2]+','
                        attr_type_list.append(param_list[x][1])
                        attr_name_list.append(param_list[x][2])
                        list_size = list_size+1
                        num_of_args = num_of_args+1
                string = string.replace('*','')
                fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")
                #This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\tvarStr = '+"'"+string+"'")
                fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\toptArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')
                fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
                fns.write("\n\n\targsList = re.split(',| ',arg)")
                fns.write("\n\targs = argsList")
                fns.write("\n\tif ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\telse:")
                command_id = '\n\t\tarr_stat_ptr = new_arr_' + 'sai_stat_id' + "_tp(len(argsList) - 1)"
                fns.write(command_id)
                fns.write("\n\t\targsLen = len(argsList)")
                fns.write("\n\t\tfor ix in range(0, argsLen - 1):")
                fns.write("\n\t\t\tarr_" + 'sai_stat_id' +"_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))");
                fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','') + "(int(args[0]), (argsLen - 1), arr_stat_ptr)"
                fns.write(fn_call)
                fns.write("\n\t\tif(ret != 0):")
                fns.write("\n\t\t\tprint('Failled with RC: %d' % ret)")
                fns.write("\n\t\telse:")
                fns.write("\n\t\t\tprint('Success')")
                delete_comamnd_id = '\n\t\tdelete_arr_' + 'sai_stat_id' + '_tp(arr_stat_ptr)'
                fns.write(delete_comamnd_id)

                if eachobjct == 'port':
                    if 'stats_attr_list' in objctmodules.keys():
                        stats_attr_list = objctmodules['stats_attr_list']
                    create = fn_list['CLEAR_STATS']
                    string =''
                    delete_string =''
                    list_flag = 0
                    spcl_flag = 0
                    index = 0
                    list_size = 0
                    num_of_args = 0
                    fns.write("\n"+"    #/*********************************************************/")
                    fns.write("\n"+"    # command for sai_clear_"+eachobjct+"_all_stats"               )
                    fns.write("\n"+"    #/*********************************************************/")
                    fns.write("\n"+"    def "+'do_'+create['name'].replace('port', 'port_all').replace('_fn','')+'(self, arg):')
                    num_of_param = create['numparam'] - 1
                    fns.write("\n\n\t'''"+create['name'].replace('port', 'port_all').replace('_fn','')+": Enter[ port_id ]'''")
                    #This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                    fns.write("\n\n\targsList = re.split(',| ',arg)")
                    fns.write("\n\targs = argsList")
                    fns.write("\n\tif (argsList[0] == ''):"+"\n\t\tprint('Invalid input, Enter [ port_id ]')")
                    fns.write("\n\telse:")
                    fn_call = "\n\t\t"+'ret = '+ create['name'].replace('port', 'port_all').replace('_fn','') + "(int(args[0]))"
                    fns.write(fn_call)
                    fns.write("\n\t\tif(ret != 0):")
                    fns.write("\n\t\t\tprint('Failled with RC: %d' % ret)")
                    fns.write("\n\t\telse:")
                    fns.write("\n\t\t\tprint('Success')")


def gen_get_stats_ext_func():

    filelist = db

    for eachfile in filelist.keys():

        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()
        fns = open('saiShellPyWrappers/'+file_name+'.py',"a+") #Open the existing py file in append mode

        objctlist = filelist[eachfile]
        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

            param_list = []
            param_count = []
            attr_count = 0
            attr_id_list = []
            attr_name_list = []
            attr_type_list = []
            fn_list = objctmodules['fn_list']
            list_string = '' #String to hold the C code for handling a list
            free_string = '' #String to hold the C code required to free the allocated memory
            decl_flag = 0

            #TODO Handling list types(array types), mac , ip6 

            if 'GET_STATS_EXT' in fn_list.keys():
                if 'stats_attr_list' in objctmodules.keys():
                    stats_attr_list = objctmodules['stats_attr_list']
                create = fn_list['GET_STATS_EXT']

                string =''
                delete_string =''
                list_flag = 0
                spcl_flag = 0
                index = 0
                list_size = 0
                num_of_args = 0
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    # command for sai_get_"+eachobjct+"_stats_ext"               )
                fns.write("\n"+"    #/*********************************************************/")
                fns.write("\n"+"    def "+'do_'+create['name'].replace('_fn','')+'(self, arg):')
                num_of_param = create['numparam']
                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_' and param_list[x][2] != '*counter_ids':
                        if (x == (num_of_param - 1)):
                            string = string+' '+param_list[x][2]
                        else:
                            string = string+' '+param_list[x][2]+','
                        attr_type_list.append(param_list[x][1])
                        attr_name_list.append(param_list[x][2])
                        list_size = list_size+1
                        num_of_args = num_of_args+1
                string = string.replace('*','')
                fns.write("\n\n\t'''"+create['name'].replace('_fn','')+': Enter[ '+string+" ]'''")
                #This part of code is taken from shellCmdsgen.py to parse the recieved arguments
                fns.write('\n\n\tvarStr = '+"'"+string+"'")
                fns.write('\n\tvarList = '+"varStr.replace(' ', '').split(',')")
                args = string.replace(' ','').split(',')
                fns.write('\n\toptArg = '+'[ ')
                strng =''
                for p in args:
                    strng = strng +"'"+ p+"'"+','
                fns.write(strng[:-1]+' ]')
                fns.write("\n\targs = ['0']*len(varList)"+"\n\ttotNumArgs = len(varList)")
                fns.write("\n\n\targsList = re.split(r'[;,\s]\s*',arg)")
                fns.write("\n\targs = argsList")
                fns.write("\n\tif ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):"+"\n\t\tprint('Invalid input, Enter [ %s ]' % varStr)")
                fns.write("\n\telse:")
                fns.write("\n\t\tcounters_list = new_arrUint64(len(argsList) - 2)")
                command_id = '\n\t\tarr_stat_ptr = new_arr_' + 'sai_stat_id' + "_tp(len(argsList) - 2)"
                fns.write(command_id)
                fns.write("\n\t\targsLen = len(argsList)")
                fns.write("\n\t\tfor ix in range(0, argsLen - 2):")
                fns.write("\n\t\t\tarr_" + 'sai_stat_id' +"_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))");
                fn_call = "\n\t\t"+'ret = '+ create['name'].replace('_fn','') + "(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)"
                fns.write(fn_call)
                fns.write("\n\t\tif(ret != 0):")
                fns.write("\n\t\t\tprint('Failled with RC: %d' % ret)")
                fns.write("\n\t\telse:")
                fns.write("\n\t\t\tresultStr = ''")
                fns.write("\n\t\t\tfor ix in range(0, argsLen - 2):")
                fns.write("\n\t\t\t\ttemp = arrUint64_getitem(counters_list, ix)")
                string = "\n\t\t\t\tresultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\\n'"
                fns.write(string)
                fns.write("\n\t\t\tprint('%s' % resultStr)")
                delete_comamnd_id = '\n\t\tdelete_arr_' + 'sai_stat_id' + '_tp(arr_stat_ptr)'
                fns.write(delete_comamnd_id)
                fns.write("\n\t\tdelete_arrUint64(counters_list)")


def gen_other_func():
    filelist = db

    done = 1
    for eachfile in filelist.keys():
        #print eachfile
        instance_name = eachfile.replace('sai','')
        file_name = 'saiShell'+instance_name.capitalize()

        fns = open('saiShellPyWrappers/'+file_name+'.py','a+') #Open the existing py file in append mode

        objctlist = filelist[eachfile]

        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]
            #Need to remove this code once generic code is implemented
            if eachfile == 'saifdb':
                string = "\n\n    def do_sai_flush_fdb_entries(self, arg):\n"
                string += "\t\t'''sai_flush_fdb_entries: Enter[  switch_id SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID,port_id SAI_FDB_FLUSH_ATTR_BV_ID,vlan_id SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,attr_type ]'''\n"
                string += "\t\tvarList = arg.split(' ')\n"
                string += "\t\tswitchId = varList[0]\n"
                string += "\t\tattr_value_arr = new_arr_sai_attribute_tp(len(varList) - 1)\n"
                string += "\t\tattr_value_ptr = new_sai_attribute_tp()\n"
                string += "\t\tcount = 0\n"
                string += "\t\tif(len(varList) <= 1):\n"
                string += "\t\t\tprint('Invalid input, Enter [  switch_id SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID,port_id SAI_FDB_FLUSH_ATTR_BV_ID,vlan_id SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,attr_type ]');\n"
                string += "\t\tfor i in range(1, len(varList)):\n"
                string += "\t\t\tattr = varList[i].replace(' ', '').split(',')\n"
                string += "\t\t\tif(len(attr) != 2):\n"
                string += "\t\t\t\tprint 'Invalid Arguemnt', varList[i]\n"
                string += "\t\t\t\tcount = 0\n"
                string += "\t\t\t\tbreak\n"
                string += "\t\t\telse:\n"
                string += "\t\t\t\tif(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID):\n"
                string += "\t\t\t\t\tattr_value_ptr.value.oid = int(attr[1])\n"
                string += "\t\t\t\telif(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_BV_ID):\n"
                string += "\t\t\t\t\tattr_value_ptr.value.oid = int(attr[1])\n"
                string += "\t\t\t\telif(eval(attr[0]) == SAI_FDB_FLUSH_ATTR_ENTRY_TYPE):\n"
                string += "\t\t\t\t\tattr_value_ptr.value.s32 = int(attr[1])\n"
                string += "\t\t\t\telse:\n"
                string += "\t\t\t\t\tprint 'Invalid attribute type:', attr[0]\n"
                string += "\t\t\t\t\tcount = 0\n"
                string += "\t\t\t\t\tbreak\n"
                string += "\t\t\t\tattr_value_ptr.id = eval(attr[0])\n"
                string += "\t\t\t\tarr_sai_attribute_tp_setitem(attr_value_arr, (i - 1),sai_attribute_tp_value(attr_value_ptr))\n"
                string += "\t\t\t\tcount = count + 1\n"
                string += "\n\t\tif(count != 0):\n"
                string += "\t\t\tret = sai_flush_fdb_entries(int(switchId), count, attr_value_arr)\n"
                string += "\t\t\tprint 'sai_flush_fdb_entries returned reason code:', ret\n"
                string += "\t\tdelete_sai_attribute_tp(attr_value_ptr)\n"
                string += "\t\tdelete_arr_sai_attribute_tp(attr_value_arr)\n"
                if done == 1:
                    done = 0
                    fns.write(string)
					
					

if __name__ == "__main__":

        gen_create_func()
        gen_set_func()
        gen_get_func()
        gen_get_stats_func()
        gen_get_stats_ext_func()
        gen_remove_func()
        gen_other_func()
        gen_alter_create_func()



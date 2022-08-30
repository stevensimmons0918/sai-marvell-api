import cPickle
import sys

f = open("db_file.txt","r")
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

####################################################################################

def processManualStrings():
    findFun = 0
    findFun1 = 0
    lines = []
    fp = open("../genScripts/saiShellCWrappers/include/saitam.h","r")
    lines = fp.read()#.splitlines():
    lines = lines.split("\n")
    for line in range(len(lines)):
         if "sai_get_tam_snapshot_stats_fn" in lines[line]:
            findFun = 1
         if(findFun == 1 and "_Inout_ uint32_t *number_of_counters," in lines[line]):
            lines[line] = "\t_Inout_ uint64_t *number_of_counters\n);"
         if(findFun == 1 and "_Inout_ sai_tam_statistic_t *statistics);" in lines[line]):
            lines[line] = " "
            findFun = 0
    fp.close()
    fp = open("../genScripts/saiShellCWrappers/include/saitam.h", "w+")
    fp.write('\n'.join(lines))
    fp.close()

####################################################################################
#Funtion for generation of "C Functions"                                           #
####################################################################################
def gen_fns():

    global callocFlg
    callocFlg = 0
#    fns = open('create_fns.c','w') 
    filelist = db

    for eachfile in filelist.keys():
	
        instance_name = eachfile.replace('sai','')
	file_name = 'saiShell'+instance_name.capitalize()
	print 'Generating saiShellCWrappers/'+file_name+'.c'
	fns = open('saiShellCWrappers/'+file_name+'.c','w')
	fns.write('#include "'+file_name+'.h"'+"\n")
	#fns.write('#include "'+'sai.h"'+"\n")
	fns.write('#include "xpSai.h"'+"\n\n")
	fns.write('#include "shellprinting.h"'+"\n\n")	
        fns.write("\n"+'#include "xpSaiValidationArrays.h"')
        fns.write("\n"+'#include "xpSaiUtil.h"'+"\n")

        if file_name == "saiShell":
            fns.write("\nsai_status_t xpSaiShellCompressAttributes(uint32_t attr_count, sai_attribute_t *attr_list, uint32_t *final_attr_count, sai_attribute_t *final_attr_list, uint32_t object_attr_count, const xpSaiAttrEntry_t * object_attr_list)")
            fns.write("\n{")
            fns.write("\n    sai_status_t ret = SAI_STATUS_SUCCESS;")
            fns.write("\n    uint32_t i = 0 ;")
            fns.write("\n    uint32_t count = 0;")
            fns.write("\n    uint32_t obj_count = 0;")
            fns.write("\n    uint32_t valid_attr_count = 0;")
            fns.write("\n    uint32_t attr_valid;")
            fns.write("\n    for (count = 0; count <attr_count; count++)")
            fns.write("\n    {")
            fns.write("\n        attr_valid = 0;")
            fns.write("\n        for (obj_count = 0; obj_count < object_attr_count; obj_count++)")
            fns.write("\n        {")
            fns.write("\n            if(attr_list[count].id == object_attr_list[obj_count].id)")
            fns.write("\n            {")
            fns.write("\n                if(object_attr_list[obj_count].conditionType != XP_SAI_NO_CONDITION)")
            fns.write("\n                {")
            fns.write("\n                    for (i=0; i < attr_count; i++)")
            fns.write("\n                    {")
            fns.write("\n                        if(object_attr_list[obj_count].conditionType == attr_list[i].id)")
            fns.write("\n                        {")
            fns.write("\n                                switch(object_attr_list[obj_count].conditionValType)")
            fns.write("\n                                {")
            fns.write("\n                                    case XP_SAI_TYPE_U8:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == (int)attr_list[i].value.u8)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_S8:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.s8)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_U16:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == (int)attr_list[i].value.u16)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_S16:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.s16)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_U32:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == (int)attr_list[i].value.u32)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_S32:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.s32)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_U64:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == (int)attr_list[i].value.u64)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_S64:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.s64)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    case XP_SAI_TYPE_BOOL:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.booldata)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                    default:")
            fns.write("\n                                        if(object_attr_list[obj_count].conditionValue == attr_list[i].value.s32)")
            fns.write("\n                                        {")
            fns.write("\n                                            attr_valid = 1;")
            fns.write("\n                                        }")
            fns.write("\n                                        break;")
            fns.write("\n                                }")
            fns.write("\n                        }")
            fns.write("\n                    }")
            fns.write("\n                }")
            fns.write("\n                else")
            fns.write("\n                    attr_valid = 1;")
            fns.write("\n            }")
            fns.write("\n        }")
            fns.write("\n        if( attr_valid ==1)")
            fns.write("\n        {")
            fns.write("\n            final_attr_list[valid_attr_count].id = attr_list[count].id;")
            fns.write("\n            final_attr_list[valid_attr_count].value = attr_list[count].value;")
            fns.write("\n            valid_attr_count++ ;")
            fns.write("\n        }")
            fns.write("\n    }")
            fns.write("\n    *final_attr_count = valid_attr_count;")
            fns.write("\n    return ret;")
	    fns.write("\n}")

	flag = 0 #This flag will be used to avoid redefining api_struct_ptr for each object in the file
        objctlist = filelist[eachfile]

        for eachobjct in objctlist.keys():
            objctmodules = objctlist[eachobjct]

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

	    ####################################################################################
	    #Script for generation of "create functions"				       #
	    ####################################################################################
            if 'CREATE' in fn_list.keys():
                create = fn_list['CREATE']
                default_string = ''
                custom_string = ''
                string = ''
                default_string = default_string+"\n\nsai_object_id_t "+create['name'].replace('_fn','')+'_default(' 
                custom_string = custom_string+"\n\nsai_object_id_t "+create['name'].replace('_fn','')+'_custom('
                num_of_param = create['numparam'] #num of parmeters in sai header excluding count and list

                if flag == 0:
                    #Defining and assigning the funtions to the funtion pointer
                    objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                    api_struct_ptr = '_xp'+objectapi[:-1]
                    z = create['struct_ptr']
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        pass
                    else:
                        fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                    flag = 1

		if 'saiShellRoute' ==  file_name:#need to handle rout seperately as it has a union in a structure in its primary arguments
		    string = string+'sai_object_id_t switch_id,'
		    string = string+' sai_object_id_t vr_id,'
		    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a union in a structure in its primary arguments
		    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'

                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

		else:
                    for x in range(num_of_param):
                	Input_list = create['params']

                    	if Input_list[x][0] == '_In_':
			    string = string+Input_list[x][1]+' '
			    string = string+Input_list[x][2]+','

                common_string = string 
                for m in range(2):
                    ip_string =''
                    string = ''
                    string = string+common_string
                    attr_count = 0
                    param_list = create['params']
                    attr_id_list = []
                    attr_type_list = []
                    attr_name_list = []
                    free_string =''
                    decl_flag = 0

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
	                    if ((m==0)and((attr_proprty['mandatory_on_create'] == 'true') or (attr_proprty['mandatory_on_create'] == 'false' and attr_proprty['valid_for_create']== 'true' and attr_proprty['valid_for_set'] == 'false') or ('saiShellFdb' == file_name and attr_proprty['valid_for_create'] == 'true')))or ((m==1)and(attr_proprty['valid_for_create'] == 'true')):
		                attr_count += 1
		                attr_id_list.append(attr_proprty['attr_id'])
			        attr_type_list.append(attr_proprty['attrib_type'][0])
			        attr_name_list.append(attr_proprty['attrib_name'])
				#considering the case when the parameter is of type list
				if '_list' in attr_proprty['attrib_type'][0]:
				    typ = attr_proprty['attrib_type'][0]
				    string = string+' '+typ+' *'+attr_proprty['attrib_name']+'_ptr,'		    
	
				elif 'sai_ip_address_t' in attr_proprty['attrib_type'][0]:
				    string = string+' sai_ip_address_t *addr_'+attr_proprty['attrib_name']+','
				    ip_string =ip_string+"\n\t"+'sai_ip_address_t '+attr_proprty['attrib_name']+';'
				    ip_string =ip_string+"\n\t"+attr_proprty['attrib_name']+'.addr_family = (sai_ip_addr_family_t)'+'addr_'+attr_proprty['attrib_name']+'->addr_family;'
				    ip_string =ip_string+"\n\t"+'memcpy(&('+attr_proprty['attrib_name']+'.addr),'+' &(addr_'+attr_proprty['attrib_name']+'->addr), 16);'

				elif 'sai_mac_t' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_mac *'+attr_proprty['attrib_name']+','

				elif 'sai_ip6_t' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_ip6 *'+attr_proprty['attrib_name']+','

				elif 'char' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_string *'+attr_proprty['attrib_name']+','
				else:
				    if eachobjct == 'acl_entry' and (m==1):
				        if "acl_field" in attr_proprty['attrib_type'][0] or "acl_action" in attr_proprty['attrib_type'][0]:
				            text = attr_proprty['attrib_type'][0].replace('_data','').replace('sai','shell')
				            string = string+' '+text+' *'+attr_proprty['attrib_name']+','
				        else:
				            string = string+' '+attr_proprty['attrib_type'][0]+' '+attr_proprty['attrib_name']+','
				    else:
				        string = string+' '+attr_proprty['attrib_type'][0]+' '+attr_proprty['attrib_name']+','
                  
                    if eachobjct == 'acl_entry' and m ==0 :
                        attr_id_list.append('SAI_ACL_ENTRY_ATTR_PRIORITY')
                        attr_type_list.append('sai_uint32_t')
                        attr_name_list.append('priority')
                        attr_count += 1
                        string = string +' sai_uint32_t priority'+','

		    string = string[:-1]
		    string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
		    string = string+")\n"
		    if eachobjct == 'acl_entry' and m ==1:
		        string = string+"{\n"
		    else:
		        string = string+"{\n\t"+'uint32_t attr_count = '+str(attr_count)+';'
                    #We should handle the case when attr_count is zero as an array of size 0 cannot be created
		    if attr_count == 0:
			string = string+("\n\t"+'sai_attribute_t *attr_list = NULL'+';')
			string = string+("\n\t"+'sai_attribute_t *final_attr_list = NULL'+';')
		    else:
		        if eachobjct == 'acl_entry' and m ==1 and (len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
			        string = string+("\n\t"+'sai_attribute_t * attr_list = (sai_attribute_t * ) calloc('+str(attr_count)+',sizeof(sai_attribute_t));')
			        string = string+("\n\t"+'if(!attr_list) return 0;')
			        string = string+("\n\t"+'sai_attribute_t * final_attr_list = (sai_attribute_t * ) calloc('+str(attr_count)+',sizeof(sai_attribute_t));')
			        string = string+("\n\t"+'if(!final_attr_list) {'+"\n\t\t"+'free (attr_list);'+"\n\t\t"+'return 0;'+"\n\t"+'}')
			        callocFlg = 1

		        elif (len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
			        string = string+("\n\t"+'sai_attribute_t * attr_list = (sai_attribute_t * ) calloc('+str(attr_count)+',sizeof(sai_attribute_t));')
			        string = string+("\n\t"+'if(!attr_list) return 0;')
			        string = string+("\n\t"+'sai_attribute_t * final_attr_list = (sai_attribute_t * ) calloc('+str(attr_count)+',sizeof(sai_attribute_t));')
			        string = string+("\n\t"+'if(!final_attr_list) {'+"\n\t\t"+'free (attr_list);'+"\n\t\t"+'return 0;'+"\n\t"+'}')
			        callocFlg = 1
		        else:
			        string = string+("\n\t"+'sai_attribute_t attr_list['+str(attr_count)+'];')
			        string = string+("\n\t"+'sai_attribute_t final_attr_list['+str(attr_count)+'];')
			        callocFlg = 0
                        string = string+("\n\t"+'int count = 0;')
                    string = string+("\n\t"+'uint32_t final_attr_count = 0;')
                    string = string+("\n\t"+'sai_object_id_t ret = 0;'+"\n")
                    if attr_count and not (eachobjct == 'acl_entry' and m ==1):
                        string = string+("\n\t"+'for (uint32_t i = 0; i < attr_count; i++)')
                        string = string+(" \n\t\t"+'attr_list[i].id = XP_SAI_OBJ_ATTR_INVALID;'+"\n")

		    if 'saiShellRoute' == file_name:#need to handle rout seperately as it has a structure in its primary arguments
		        string = string+("\n\t"+'sai_route_entry_t *route_entry = NULL;')
		        string = string+("\n\troute_entry = (sai_route_entry_t*) calloc(1,sizeof(sai_route_entry_t));")
		        string = string+("\n\t"+'if(!route_entry) {')
		        string = string+("\n\t\t"+'if(attr_list) free(attr_list);')
		        string = string+("\n\t\t"+'if(final_attr_list) free(final_attr_list);')
		        string = string+("\n\t\t"+'return ret; }')
		        string = string+("\n\t"+'route_entry->switch_id = switch_id;')
		        string = string+("\n\t"+'route_entry->vr_id = vr_id;')
		        string = string+("\n\t"+'route_entry->destination.addr_family = (sai_ip_addr_family_t) ip_addr_family;')
		        string = string+("\n\t"+'memcpy(&(route_entry->destination.addr), &(ipaddr->addr) , 16);')
		        string = string+("\n\t"+'memcpy(&(route_entry->destination.mask), &(mask->addr) , 16);')

		    elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
			string = string+("\n\t"+'sai_neighbor_entry_t *neighbor_entry = NULL;')
			string = string+("\n\tneighbor_entry = (sai_neighbor_entry_t*) calloc(1,sizeof(sai_neighbor_entry_t));")
			string = string+("\n\t"+'if(!neighbor_entry) {')
			string = string+("\n\t\t"+'if(attr_list) free(attr_list);')
			string = string+("\n\t\t"+'if(final_attr_list) free(final_attr_list);')
			string = string+("\n\t\t"+'return ret; }')
			string = string+("\n\t"+'neighbor_entry->switch_id = switch_id;')
			string = string+("\n\t"+'neighbor_entry->rif_id = rif_id;')
			string = string+("\n\t"+'neighbor_entry->ip_address.addr_family = (sai_ip_addr_family_t) ip_address->addr_family ;')
			string = string+("\n\t"+'memcpy(&(neighbor_entry->ip_address.addr), &(ip_address->addr) , 16);')

                    elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a structure in its primary arguments
                        string = string+("\n\t"+'sai_l2mc_entry_t *l2mc_entry = NULL;')
                        string = string+("\n\tl2mc_entry = (sai_l2mc_entry_t*) calloc(1,sizeof(sai_l2mc_entry_t));")
                        string = string+("\n\t"+'if(!l2mc_entry) {')
                        string = string+("\n\t\t"+'if(attr_list) free(attr_list);')
                        string = string+("\n\t\t"+'if(final_attr_list) free(final_attr_list);')
                        string = string+("\n\t\t"+'return ret; }')
                        string = string+("\n\t"+'l2mc_entry->switch_id = switch_id;')
                        string = string+("\n\t"+'l2mc_entry->bv_id = bv_id;')
                        string = string+("\n\t"+'l2mc_entry->type = entry_type;')
                        string = string+("\n\t"+'l2mc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family;')
                        string = string+("\n\t"+'l2mc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family;')
                        string = string+("\n\t"+'if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                        string = string+(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                        string = string+("\n\t"+'else')
                        string = string+(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                        string = string+("\n\t"+'if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                        string = string+(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                        string = string+("\n\t"+'else')
                        string = string+(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

                    elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a structure in its primary arguments
                        string = string+("\n\t"+'sai_ipmc_entry_t *ipmc_entry = NULL;')
                        string = string+("\n\tipmc_entry = (sai_ipmc_entry_t*) calloc(1,sizeof(sai_ipmc_entry_t));")
                        string = string+("\n\t"+'if(!ipmc_entry) {')
                        string = string+("\n\t\t"+'if(attr_list) free(attr_list);')
                        string = string+("\n\t\t"+'if(final_attr_list) free(final_attr_list);')
                        string = string+("\n\t\t"+'return ret; }')
                        string = string+("\n\t"+'ipmc_entry->switch_id = switch_id;')
                        string = string+("\n\t"+'ipmc_entry->vr_id = vr_id;')
                        string = string+("\n\t"+'ipmc_entry->type = entry_type;')
                        string = string+("\n\t"+'ipmc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family;')
                        string = string+("\n\t"+'ipmc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family;')
                        string = string+("\n\t"+'if (ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                        string = string+(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                        string = string+("\n\t"+'else')
                        string = string+(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                        string = string+("\n\t"+'if (ipmc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                        string = string+(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                        string = string+("\n\t"+'else')
                        string = string+(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

	            string = string+list_string
		    string = string+ip_string

		    for j in range(num_of_param):
			if param_list[j][0] == '_Out_':
			    param_list[j][2] = param_list[j][2].strip('_out')+'_out' #To avoid the case of parameter shadowing we modify the variable name by adding '_out'
			    if '*' in param_list[j][2]: #Allocate a memory from heap to store the out parameter 
				string = string+"\n\t"+param_list[j][1]+' '+param_list[j][2]+' = ('+param_list[j][1]+'*) '+'calloc(1,sizeof('+param_list[j][1]+')) ;'
				string = string+("\n\t"+'if (!'+param_list[j][2].strip('*')+') {')
				string = string+("\n\t\t"+'if (attr_list) free(attr_list);')
				string = string+("\n\t\t"+'if (final_attr_list) free(final_attr_list);')
				string = string+("\n\t\t"+'return ret; }')
				free_string = free_string+"\n\t"+'free('+param_list[j][2].strip('*')+');'
			    else:
				string = string+("\n\t"+param_list[j][1]+' '+param_list[j][2]+' = 0;')
		
		    for i in range(attr_count):
			if attr_type_list[i] in ub.keys():		
			    if ub[attr_type_list[i]] == "mac" or ub[attr_type_list[i]] == "ip6" or ub[attr_type_list[i]] == "chardata":
			        string = string+("\n\t"+'int j = 0 ;')
				break
		    string = string+("\n")
		    if(eachobjct == 'qos_map'):
		        attr_type_list.reverse()
		        attr_id_list.reverse()
		        attr_name_list.reverse()

		    for i in range(attr_count):
    
                        if eachobjct == 'acl_entry' and (m == 1) and (ub[attr_type_list[i]] == 'aclaction' or ub[attr_type_list[i]] == 'aclfield'):
                                string = string+("\n\tif("+attr_name_list[i]+"->enable != 0)")
                                string = string+("\n\t{")
                                string = string+("\n\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                        else:
                            if 'saiShellAcl' == file_name:
                                if "sai_create_acl_table_custom" in custom_string:
                                    if ('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6' == attr_id_list[i]):
                                        string = string+("\n\tif (field_dst_ipv6 == 1)\n")
                                        string = string+("\t{\n")
                                        string = string+("\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                    elif ('SAI_ACL_TABLE_ATTR_FIELD_SRC_IP' == attr_id_list[i]):
                                        string = string+("\n\tif (field_src_ip == 1)\n")
                                        string = string+("\t{\n")
                                        string = string+("\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                    elif ('SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL' == attr_id_list[i]):
                                        string = string+("\n\tif (field_ipv6_flow_label == 1)\n")
                                        string = string+("\t{\n")
                                        string = string+("\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                    elif ('SAI_ACL_TABLE_ATTR_FIELD_DST_IP' == attr_id_list[i]):
                                        string = string+("\n\tif (field_dst_ip == 1)\n")
                                        string = string+("\t{\n")
                                        string = string+("\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                    elif ('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6' == attr_id_list[i]):
                                        string = string+("\n\tif (field_src_ipv6 == 1)\n")
                                        string = string+("\t{\n")
                                        string = string+("\t\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                    else:
                                        string = string+("\n\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                                else:
                                    string = string+("\n\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
                            else:
                                string = string+("\n\t"+'attr_list[count].id = '+ attr_id_list[i]+';')
			if attr_type_list[i] in ub.keys():
			    if ub[attr_type_list[i]] == "mac": #If the parameter is an array(mac and ip6) each element have to be copied by index
				string = string+("\n\t"+'for(j=0 ;j<6 ; j++){ ')
				string = string+("\n\t\t"+'attr_list[count].value.mac[j] = '+attr_name_list[i]+'->mac[j];}')
				string = string+("\n\t"+'count++;')
			    elif  ub[attr_type_list[i]] == "ip6":		
				string = string+("\n\t"+'for(j=0 ;j<16 ; j++){ ')
				string = string+("\n\t\t"+'attr_list[count].value.ip6[j] = '+attr_name_list[i]+'->addr[j];}')
				string = string+("\n\t"+'count++;')
			    elif  ub[attr_type_list[i]] == "chardata":
				string = string+("\n\t"+'for(j=0 ;j<30 ; j++){ ')
				string = string+("\n\t\t"+'attr_list[count].value.chardata[j] = '+attr_name_list[i]+'->name[j];}')
				string = string+("\n\t"+'count++;')
			    # As per sai header qosmap is also one type of list
			    elif 'list' in ub[attr_type_list[i]] or 'qosmap' in ub[attr_type_list[i]] or 'tunnelmap' in ub[attr_type_list[i]]:
                                string = string+("\n\tif("+attr_name_list[i]+'_ptr->count != 0){ ')
				string = string+("\n\t\tattr_list[count].value."+ub[attr_type_list[i]]+'.count = '+attr_name_list[i]+'_ptr->count;')
				string = string+("\n\t\tattr_list[count++].value."+ub[attr_type_list[i]]+'.list = '+attr_name_list[i]+'_ptr->list;}')
			    elif eachobjct == 'acl_entry' and (m == 1):
				if ub[attr_type_list[i]] == 'aclaction':
					string = string+("\n\t\t"+'memcpy(&attr_list[count++].value.'+ub[attr_type_list[i]]+', '+attr_name_list[i]+' ,sizeof(shell_acl_action_t));')
					string  = string+("\n\t}") 
				elif ub[attr_type_list[i]] == 'aclfield':
					string = string+("\n\t\t"+'memcpy(&attr_list[count++].value.'+ub[attr_type_list[i]]+', '+attr_name_list[i]+' ,sizeof(shell_acl_field_t));')
					string  = string+("\n\t}") 
				else:
					string = string+("\n\t"+'attr_list[count++].value.'+ub[attr_type_list[i]]+' = '+attr_name_list[i]+';')
			    else:
                                if 'saiShellAcl' == file_name:
                                    if "sai_create_acl_table_custom" in custom_string:
                                        if ('SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6' == attr_id_list[i]) or ('SAI_ACL_TABLE_ATTR_FIELD_SRC_IP' == attr_id_list[i]) or ('SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL' == attr_id_list[i]) or ('SAI_ACL_TABLE_ATTR_FIELD_DST_IP' == attr_id_list[i]) or ('SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6' ==  attr_id_list[i]):
				            string = string+("\n\t\t"+'attr_list[count++].value.'+ub[attr_type_list[i]]+' = '+attr_name_list[i]+';')
                                            string = string+("\n\t}")
                                        else:
				            string = string+("\n\t"+'attr_list[count++].value.'+ub[attr_type_list[i]]+' = '+attr_name_list[i]+';')
                                    else:
				        string = string+("\t"+'attr_list[count++].value.'+ub[attr_type_list[i]]+' = '+attr_name_list[i]+';')
                                else:
                                    string = string+("\n\t"+'attr_list[count++].value.'+ub[attr_type_list[i]]+' = '+attr_name_list[i]+';')
			else:
			    string = string+("\n\t"+'attr_list[count++].value.s32'+' = '+attr_name_list[i]+';')


		    #objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
		    #api_struct_ptr = '_xp'+objectapi[:-1]
                    

		    struct_elemnts = apistruct[create['struct_ptr']]
                    apipartname = eachobjct
		    if eachobjct == 'acl_entry' and m == 1:
		        string = string+("\n\n\t"+"xpSaiShellCompressAttributes(count, attr_list, &final_attr_count, final_attr_list,"+apipartname.upper()+"_VALIDATION_ARRAY_SIZE, "+apipartname+"_attribs);")
                    elif 'saiShellAcl' == file_name:
                        if "sai_create_acl_table_custom" in custom_string:
		            string = string+("\n\n\t"+"xpSaiShellCompressAttributes(count, attr_list, &final_attr_count, final_attr_list,"+apipartname.upper()+"_VALIDATION_ARRAY_SIZE, "+apipartname+"_attribs);")
                        else:
		            string = string+("\n\n\t"+"xpSaiShellCompressAttributes(attr_count, attr_list, &final_attr_count, final_attr_list,"+apipartname.upper()+"_VALIDATION_ARRAY_SIZE, "+apipartname+"_attribs);")
		    else:
		        string = string+("\n\n\t"+"xpSaiShellCompressAttributes(attr_count, attr_list, &final_attr_count, final_attr_list,"+apipartname.upper()+"_VALIDATION_ARRAY_SIZE, "+apipartname+"_attribs);")
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        string = string+('\n\n\t'+'if((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
                        string = string+'\n\t{'
                        string = string+('\n\n\t\t'+'(('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                    else:
			if "_xpSaiBridgeApi" == api_struct_ptr:
	                        string = string+("\n\n\tsai_status_t saiStatus;\n\t"+'saiStatus = '+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')
			else:
	                        string = string+("\n\n\t"+''+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')
		
		    tmp_string = ''
		    for x in range(num_of_param):
			tmp_string = tmp_string+param_list[x][2]+', '
		    tmp_string = tmp_string+' final_attr_count, '+' final_attr_list );'
			
		    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		        tmp_string = tmp_string+'\n\t}'
		    string = string +(tmp_string.replace('*',''))

		    for j in range(num_of_param):

			if param_list[j][0] == '_Out_':
				if "_xpSaiBridgeApi" == api_struct_ptr and (len(sys.argv) != 2):
				    string = string+('\n\tif (saiStatus == SAI_STATUS_SUCCESS) printf("'+param_list[j][2].replace('*','')+' = %" PRIu64 "\\n",'+param_list[j][2]+');')
				else:
				    string = string+('\n\tprintf("'+param_list[j][2].replace('*','')+' = %" PRIu64 "\\n",'+param_list[j][2]+');')
				string = string+('\n\tret = '+	param_list[j][2]+';')

                    string = string+("\n"+free_string)

                    if 'saiShellL2mc' == file_name:#need to free l2mc seperately as it has a structure in its primary arguments
                        string = string+("\n\t"+'free(l2mc_entry);')
                    elif 'saiShellIpmc' == file_name:#need to free ipmc seperately as it has a structure in its primary arguments
                        string = string+("\n\t"+'free(ipmc_entry);')
                    elif 'saiShellRoute' == file_name:
                        string = string+("\n\t"+'free(route_entry);')
                    elif 'saiShellNeighbor' == file_name:
                        string = string+("\n\t"+'free(neighbor_entry);')
		    if callocFlg == 1:
		        string = string+("\n\t"+'free(attr_list);')
		        string = string+("\n\t"+'free(final_attr_list);')
		        callocFlg = 0
		    string = string+("\n\n\treturn ret;\n}")

		    if(m==0):
			default_string = default_string+string
		    elif(m==1):
			custom_string = custom_string + string

		    param_list = []

		fns.write(default_string)
		fns.write(custom_string)
	    
	    ####################################################################################
            #Script for generation of "SET functions"                                          #
            ####################################################################################
	    
	    #TODO Handling list types(array types), mac , ip6 

	    if 'SET' in fn_list.keys():
                create = fn_list['SET']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

                num_of_param = create['numparam'] #num of parmeters in sai header excluding count and list

                if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                            pass
                        else:
                            fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1
		if 'saiShellRoute' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'
		
		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

                if eachobjct == 'acl_entry':
                    string = string +'uint32_t attr_id, sai_attribute_value_t value, int aclfieldflag, shell_acl_field_t *aclfield, int aclactionflag, shell_acl_action_t *aclaction){'
                else:
                    string = string +'uint32_t attr_id, sai_attribute_value_t value, int ipaddr_flag, sai_ip_address_t *ip_addr){' 
		string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
			
		fns.write(string)
		fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
		fns.write("\n\t"+'sai_attribute_t attr;')
		fns.write("\n\n\t"+'attr.id = (uint32_t)attr_id ;'+"\n")
		#fns.write("\n\t"+'attr.value = value ;'+"\n")

		if 'saiShellRoute' == file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_route_entry_t *route_entry = NULL;')
                    fns.write("\n\troute_entry = (sai_route_entry_t*) calloc(1,sizeof(sai_route_entry_t));")
                    fns.write("\n\t"+'if(!route_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'route_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'route_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'route_entry->destination.addr_family = (sai_ip_addr_family_t) ip_addr_family;')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.addr), &(ipaddr->addr) , 16);')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.mask), &(mask->addr) , 16);')

		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_neighbor_entry_t *neighbor_entry = NULL;')
                    fns.write("\n\tneighbor_entry = (sai_neighbor_entry_t*) calloc(1,sizeof(sai_neighbor_entry_t));")
                    fns.write("\n\t"+'if(!neighbor_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'neighbor_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'neighbor_entry->rif_id = rif_id;')
                    fns.write("\n\t"+'neighbor_entry->ip_address.addr_family = (sai_ip_addr_family_t) ip_address->addr_family ;')
                    fns.write("\n\t"+'memcpy(&(neighbor_entry->ip_address.addr), &(ip_address->addr) , 16);')

                if eachobjct == 'acl_entry':
                    fns.write("\n\tif (aclfieldflag !=0){")
                    fns.write("\n\t\tmemcpy(&attr.value.aclfield, aclfield, sizeof(shell_acl_field_t));}")
                    fns.write("\n\telse if (aclactionflag !=0){")
                    fns.write("\n\t\tmemcpy(&attr.value.aclaction, aclaction, sizeof(shell_acl_action_t));}")
                    fns.write("\n\telse")
                    fns.write("\n\tattr.value = value;")

                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_l2mc_entry_t *l2mc_entry = NULL;')
                    fns.write("\n\tl2mc_entry = (sai_l2mc_entry_t*) calloc(1,sizeof(sai_l2mc_entry_t));")
                    fns.write("\n\t"+'if(!l2mc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'l2mc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'l2mc_entry->bv_id = bv_id;')
                    fns.write("\n\t"+'l2mc_entry->type = entry_type;')
                    fns.write("\n\t"+'l2mc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'l2mc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')
                    fns.write("\n")
                    fns.write("\n\tif(ipaddr_flag != 0){\n\t\tmemcpy(&attr.value.ipaddr, ip_addr, sizeof(sai_ip_address_t));}")
                    fns.write("\n\telse\n\t\tattr.value = value;")

                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_ipmc_entry_t *ipmc_entry = NULL;')
                    fns.write("\n\tipmc_entry = (sai_ipmc_entry_t*) calloc(1,sizeof(sai_ipmc_entry_t));")
                    fns.write("\n\t"+'if(!ipmc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'ipmc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'ipmc_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'ipmc_entry->type = entry_type;')
                    fns.write("\n\t"+'ipmc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'ipmc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (ipmc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')
                    fns.write("\n")
                    fns.write("\n\tif(ipaddr_flag != 0){\n\t\tmemcpy(&attr.value.ipaddr, ip_addr, sizeof(sai_ip_address_t));}")
                    fns.write("\n\telse\n\t\tattr.value = value;")

                else:
		    fns.write("\n")
		    fns.write("\n\tif(ipaddr_flag != 0){\n\t\tmemcpy(&attr.value.ipaddr, ip_addr, sizeof(sai_ip_address_t));}")
                    fns.write("\n\telse\n\t\tattr.value = value;")
                
		struct_elemnts = apistruct[create['struct_ptr']]
		if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    fns.write('\n\tif((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
		    fns.write('\n\t{')
		    fns.write('\n\t\tret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
		else:
		    fns.write("\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')

		string =''
		
		for x in range(num_of_param):
                    param_list = create['params']

                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][2]+','


		if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    string = string+'&attr));'+"\n\t}"
		else:
		    string = string+'&attr));'
                if 'saiShellL2mc' == file_name:#need to free l2mc seperately as it has a structure in its primary arguments
                    string = string + '\n\tfree(l2mc_entry);'
                elif 'saiShellIpmc' == file_name:#need to free ipmc seperately as it has a structure in its primary arguments
                    string = string + '\n\tfree(ipmc_entry);'
                elif 'saiShellRoute' == file_name:
                    string = string+("\n\t"+'free(route_entry);')
                elif 'saiShellNeighbor' == file_name:
                    string = string+("\n\t"+'free(neighbor_entry);')
		string = string.replace('*','')
		if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    string = string.replace('_api_t','_api_t*')
		#fns.write(string)

                fns.write(string.replace('*',''))
                fns.write("\n\n\treturn ret;\n}")

	    #################################################################################
            #Script for generation of "GET functions"                                          #
            ####################################################################################
	    
	    #TODO Handling list types(array types), mac , ip6 

	    if 'GET' in fn_list.keys():
		create = fn_list['GET']
                if 'saiShellSwitch' == file_name:
                    string = "\n\nsai_object_id_t "+create['name'].replace('_fn','')+'('
                else:
                    string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

                num_of_param = create['numparam']
	
		if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                            pass
                        else:
                            fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1

		if 'saiShellRoute' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

		string = string +' sai_'+eachobjct+'_attr_t attr_id , uint32_t list_count, sai_attribute_t *out_attr)'+"\n{"
		string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
	
		fns.write(string)
		if 'saiShellRoute' == file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_route_entry_t *route_entry = NULL;')
                    fns.write("\n\troute_entry = (sai_route_entry_t*) calloc(1,sizeof(sai_route_entry_t));")
                    fns.write("\n\t"+'if(!route_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'route_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'route_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'route_entry->destination.addr_family = (sai_ip_addr_family_t) ip_addr_family;')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.addr), &(ipaddr->addr) , 16);')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.mask), &(mask->addr) , 16);')
		
		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_neighbor_entry_t *neighbor_entry = NULL;')
                    fns.write("\n\tneighbor_entry = (sai_neighbor_entry_t*) calloc(1,sizeof(sai_neighbor_entry_t));")
                    fns.write("\n\t"+'if(!neighbor_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'neighbor_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'neighbor_entry->rif_id = rif_id;')
                    fns.write("\n\t"+'neighbor_entry->ip_address.addr_family = (sai_ip_addr_family_t) ip_address->addr_family ;')
                    fns.write("\n\t"+'memcpy(&(neighbor_entry->ip_address.addr), &(ip_address->addr) , 16);')
                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_l2mc_entry_t *l2mc_entry = NULL;')
                    fns.write("\n\tl2mc_entry = (sai_l2mc_entry_t*) calloc(1,sizeof(sai_l2mc_entry_t));")
                    fns.write("\n\t"+'if(!l2mc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'l2mc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'l2mc_entry->bv_id = bv_id;')
                    fns.write("\n\t"+'l2mc_entry->type = entry_type;')
                    fns.write("\n\t"+'l2mc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'l2mc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_ipmc_entry_t *ipmc_entry = NULL;')
                    fns.write("\n\tipmc_entry = (sai_ipmc_entry_t*) calloc(1,sizeof(sai_ipmc_entry_t));")
                    fns.write("\n\t"+'if(!ipmc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'ipmc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'ipmc_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'ipmc_entry->type = entry_type;')
                    fns.write("\n\t"+'ipmc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'ipmc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (ipmc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

		fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
		fns.write("\n\t"+'sai_attribute_t attr;')
		fns.write("\n\t"+'uint32_t count = 1;')
		fns.write("\n\t"+'attr.id = (uint32_t) attr_id;')
		fns.write("\n\t"+'char *type;')
		fns.write("\n\t"+'char attr_type[100] = {'+"'\\0'"+'};') 
		fns.write("\n\t"+'type = attr_type;')
                if eachobjct == 'acl_entry':
                    fns.write("\n\t"+'char *maintype;')
                    fns.write("\n\t"+'char main_attr_type[100] = {'+"'\\0'"+'};')
                    fns.write("\n\t"+'maintype = main_attr_type;')
		struct_elemnts = apistruct[create['struct_ptr']]
		fns.write("\n\t"+'void* list_ptr=NULL;')
                if eachobjct == 'acl_entry':
                    fns.write("\n\t"+'void* list_ptr1=NULL;')
                    fns.write("\n\t"+'void* list_ptr2=NULL;')
		if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    string = "\n\tif((("+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))"
		    string = string+"\n\t{"
		    string = string+("\n\t\t"+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
#		    print "    DEBUG   :" ,string
		else:
		    string = "\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'('
#		    print "    DEBUG   ::" ,string

		for x in range(num_of_param):
                    param_list = create['params']

                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][2]+','

		string = string+'count, &attr));'
		if (len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    string = string+'\n\t}'
		fns.write("\n\n\t"+'switch(attr_id)\n\t{') 			
		
                if eachobjct == 'acl_entry':

                    for attr in attr_list.keys():
                        attr_proprty = attr_list[attr]
                        fns.write("\n\t\t"+'case '+ attr_proprty['attr_id']+':\n\t\t')
                        if attr_proprty['attrib_type'][0] in ub.keys():
                            if attr_proprty['attrib_type'][0] == 'sai_acl_field_data_t':
                                fns.write("\n\t\t\tstrcpy(type ,"+'"'+attr_proprty['attrib_type'][1]+'") ;')
                                fns.write("\n\t\t\tstrcpy(maintype ,"+'"'+attr_proprty['attrib_type'][0]+'") ;')
                                if attr_proprty['attrib_type'][1] == 'sai_object_list_t':
                                    list_struct = lb[attr_proprty['attrib_type'][1]]
                                    fns.write("\n\t\t\tlist_ptr = ("+list_struct['list'][0]+") calloc(list_count, sizeof("+list_struct['list'][0].strip('*')+"));")
                                    fns.write("\n\t\t\tif(!list_ptr) return SAI_STATUS_FAILURE;")
                                    fns.write("\n\t\t\tattr.value.aclfield.data."+ub[attr_proprty['attrib_type'][1]]+'.list = ('+list_struct['list'][0]+')list_ptr;')
                                    fns.write("\n\t\t\tattr.value.aclfield.data."+ub[attr_proprty['attrib_type'][1]]+'.count = list_count ;')
                                elif attr_proprty['attrib_type'][1] == 'sai_u8_list_t':
                                    list_struct = lb[attr_proprty['attrib_type'][1]]
                                    fns.write("\n\t\t\tlist_ptr2 = ("+list_struct['list'][0]+") calloc(list_count, sizeof("+list_struct['list'][0].strip('*')+"));")
                                    fns.write("\n\t\t\tif(!list_ptr2) return SAI_STATUS_FAILURE;")
                                    fns.write("\n\t\t\tattr.value.aclfield.mask."+ub[attr_proprty['attrib_type'][1]]+'.list = ('+list_struct['list'][0]+')list_ptr2;')
                                    fns.write("\n\t\t\tattr.value.aclfield.mask."+ub[attr_proprty['attrib_type'][1]]+'.count = list_count ;')
                                    fns.write("\n\t\t\tlist_ptr1 = ("+list_struct['list'][0]+") calloc(list_count, sizeof("+list_struct['list'][0].strip('*')+"));")
                                    fns.write("\n\t\t\tif(!list_ptr1){ free (list_ptr2); return SAI_STATUS_FAILURE; }")
                                    fns.write("\n\t\t\tattr.value.aclfield.data."+ub[attr_proprty['attrib_type'][1]]+'.list = ('+list_struct['list'][0]+')list_ptr1;')
                                    fns.write("\n\t\t\tattr.value.aclfield.data."+ub[attr_proprty['attrib_type'][1]]+'.count = list_count ;')

                            elif attr_proprty['attrib_type'][0] == 'sai_acl_action_data_t':
                                fns.write("\n\t\t\tstrcpy(type ,"+'"'+attr_proprty['attrib_type'][1]+'") ;')
                                fns.write("\n\t\t\tstrcpy(maintype ,"+'"'+attr_proprty['attrib_type'][0]+'") ;')
                                if attr_proprty['attrib_type'][1] == 'sai_object_list_t':
                                    list_struct = lb[attr_proprty['attrib_type'][1]]
                                    fns.write("\n\t\t\tlist_ptr = ("+list_struct['list'][0]+") calloc(list_count, sizeof("+list_struct['list'][0].strip('*')+"));")
                                    fns.write("\n\t\t\tif(!list_ptr) return SAI_STATUS_FAILURE;")
                                    fns.write("\n\t\t\tattr.value.aclaction.parameter."+ub[attr_proprty['attrib_type'][1]]+'.list = ('+list_struct['list'][0]+')list_ptr;')
                                    fns.write("\n\t\t\tattr.value.aclaction.parameter."+ub[attr_proprty['attrib_type'][1]]+'.count = list_count ;')
                            else:
                                fns.write("\n\t\t\tstrcpy(type ,"+'"'+attr_proprty['attrib_type'][0]+'") ;')
                        else:
                            fns.write("\n\t\t\tstrcpy(type ,"+'"'+attr_proprty['attrib_type'][0]+'") ;')
       
                        fns.write("\n\t\t\tbreak;")	
                else:
                    for attr in attr_list.keys():

                        attr_proprty = attr_list[attr]
                        fns.write("\n\t\t"+'case '+ attr_proprty['attr_id']+':\n\t\t')
                        fns.write("\n\t\t\tstrcpy(type ,"+'"'+attr_proprty['attrib_type'][0]+'") ;')
                        if '_list_t' in attr_proprty['attrib_type'][0]:
                            list_struct = lb[attr_proprty['attrib_type'][0]]
                            fns.write("\n\t\t\tlist_ptr = ("+list_struct['list'][0]+") calloc(list_count, sizeof("+list_struct['list'][0].strip('*')+"));")
                            fns.write("\n\t\t\tif(!list_ptr) return SAI_STATUS_FAILURE;")
                            fns.write("\n\t\t\tattr.value."+ub[attr_proprty['attrib_type'][0]]+'.list = ('+list_struct['list'][0]+')list_ptr;')
                            fns.write("\n\t\t\tattr.value."+ub[attr_proprty['attrib_type'][0]]+'.count = list_count ;')
                        fns.write("\n\t\t\tbreak;")


		fns.write('\n\t\tdefault:\n\t\t\tprintf("Invalid attribute-Id Entered, Please enter a Valid attr-id\\n");\n\t}')		
		string = string.replace('*','')
		if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		    string = string.replace('_api_t','_api_t*')
		fns.write(string)
                fns.write("\n\t"+'if(ret == 0)') #Print only if get is succes

                if eachobjct == 'acl_entry':
                    fns.write("\n\tacl_print_attribute("+'maintype, type ,attr.value, out_attr);')
                else:
                    fns.write("\n\tprint_attribute("+'type ,attr.value, out_attr);')
		fns.write("\n\n\tif(list_ptr!= NULL)\n\t\tfree(list_ptr);")
                if eachobjct == 'acl_entry':
                    fns.write("\n\n\tif(list_ptr1!= NULL)\n\t\tfree(list_ptr1);")
                    fns.write("\n\n\tif(list_ptr2!= NULL)\n\t\tfree(list_ptr2);")

                if 'saiShellL2mc' == file_name:#need to free l2mc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'free(l2mc_entry);')
                elif 'saiShellIpmc' == file_name:#need to free ipmc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'free(ipmc_entry);')
                elif 'saiShellRoute' == file_name:
                    fns.write("\n\t"+'free(route_entry);')
                elif 'saiShellNeighbor' == file_name:
                    fns.write("\n\t"+'free(neighbor_entry);')

		if 'saiShellSwitch' == file_name:
			fns.write("\n\n\treturn attr.value.oid;"+"\n}")
		else:
			fns.write("\n\n\treturn ret;"+"\n}")
		

	    if 'GET_STATS' in fn_list.keys():
                create = fn_list['GET_STATS']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']
                if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1


                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        string = string+param_list[x][2]+','
                string = string + "uint64_t *counters)"+"\n{"
                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
                fns.write(string)
                fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
                struct_elemnts = apistruct[create['struct_ptr']]
                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                   fns.write("\n\tif((("+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
                   fns.write("\n\t{")
                   fns.write("\n\t\t"+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                else:
                   fns.write("\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')

                string =''

                for x in range(num_of_param):
                    param_list = create['params']

                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][2]+','


                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                    string = string+'counters));'+'\n\t}'+'\n\n\treturn ret;'+'\n}'
                else:
                    string = string+'counters));'+"\n\n\treturn ret;"+"\n}"
                fns.write(string.replace('*',''))


	    if 'GET_STATS_EXT' in fn_list.keys():
                create = fn_list['GET_STATS_EXT']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']
                if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1


                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        string = string+param_list[x][2]+','
                string = string + "uint64_t *counters)"+"\n{"
                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
                fns.write(string)
                fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
                struct_elemnts = apistruct[create['struct_ptr']]
                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                   fns.write("\n\tif((("+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
                   fns.write("\n\t{")
                   fns.write("\n\t\t"+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                else:
                   fns.write("\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')

                string =''

                for x in range(num_of_param):
                    param_list = create['params']

                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][2]+','


                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                    string = string+'counters));'+'\n\t}'+'\n\n\treturn ret;'+'\n}'
                else:
                    string = string+'counters));'+"\n\n\treturn ret;"+"\n}"
                fns.write(string.replace('*',''))


	    if 'CLEAR_STATS' in fn_list.keys():
                create = fn_list['CLEAR_STATS']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']
                if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1


                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        if x == (num_of_param - 1):
                            string = string+param_list[x][2]+')\n{'
                        else:
                            string = string+param_list[x][2]+','
                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
                fns.write(string)
                fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
                struct_elemnts = apistruct[create['struct_ptr']]
                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                   fns.write("\n\tif((("+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
                   fns.write("\n\t{")
                   fns.write("\n\t\t"+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                else:
                   fns.write("\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')

                string =''

                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        if x == (num_of_param - 1):
                            string = string+param_list[x][2]+'));'
                        else:
                            string = string+param_list[x][2]+','


                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                   string = string + "\n\t}\n\n\treturn ret;"+"\n}"
                else:
                   string = string + "\n\n\treturn ret;"+"\n}"
                fns.write(string.replace('*',''))

                if eachobjct == 'port': 
                    create = fn_list['CLEAR_STATS']
                    create_name = create['name'].replace('port','port_all')
                    string = "\n\nsai_status_t "+create_name.replace('_fn','')+'(sai_object_id_t port_id)\n{'
                    fns.write(string)
                    fns.write("\n\tsai_status_t ret = SAI_STATUS_SUCCESS;")
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                         string = "\n\tif((("+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))"
                         string = string + "\n\t{"
                         string = string + "\n\t\t"+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create_name]+'(port_id));'
                         string = string + "\n\t}"
                    else:
                         string = "\n\t"+'ret = '+api_struct_ptr+'->'+struct_elemnts[create_name]+'(port_id);'
                    fns.write(string)
                    fns.write("\n\treturn ret;\n}")

	    ####################################################################################
            #Script for generation of "REMOVE functions"                                          #
            ####################################################################################

            #TODO Handling list types(array types), mac , ip6 
	    if 'REMOVE' in fn_list.keys():
                string =""
                create = fn_list['REMOVE']
                if 'switch' in create['name']:
                    string = "\n\nvoid "+create['name'].replace('_fn','')+'('
                else:
                    string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

                num_of_param = create['numparam']

                if flag == 0:
                        #Defining and assigning the funtions to the funtion pointer
                        objectapi = create['struct_ptr'].replace('_',' ').title().replace(' ','')
                        api_struct_ptr = '_xp'+objectapi[:-1]
                        z = create['struct_ptr']
                        if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                            pass
                        else:
                            fns.write('static '+z+'* '+api_struct_ptr+' = ('+z+'*) '+'xpSaiApiTableArr['+ apienums[z]+"];")
                        flag = 1
		if 'saiShellRoute' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'
		
		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

                string = string[:-1]
		string = string+')'+"\n{"
		string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
		fns.write(string)
		if 'saiShellRoute' == file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_route_entry_t *route_entry = NULL;')
                    fns.write("\n\troute_entry = (sai_route_entry_t*) calloc(1,sizeof(sai_route_entry_t));")
                    fns.write("\n\t"+'if(!route_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'route_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'route_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'route_entry->destination.addr_family = (sai_ip_addr_family_t) ip_addr_family;')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.addr), &(ipaddr->addr) , 16);')
                    fns.write("\n\t"+'memcpy(&(route_entry->destination.mask), &(mask->addr) , 16);')

		elif 'saiShellNeighbor' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_neighbor_entry_t *neighbor_entry = NULL;')
                    fns.write("\n\tneighbor_entry = (sai_neighbor_entry_t*) calloc(1,sizeof(sai_neighbor_entry_t));")
                    fns.write("\n\t"+'if(!neighbor_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'neighbor_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'neighbor_entry->rif_id = rif_id;')
                    fns.write("\n\t"+'neighbor_entry->ip_address.addr_family = (sai_ip_addr_family_t) ip_address->addr_family ;')
                    fns.write("\n\t"+'memcpy(&(neighbor_entry->ip_address.addr), &(ip_address->addr) , 16);')

                elif 'saiShellL2mc' == file_name:#need to handle l2mc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_l2mc_entry_t *l2mc_entry = NULL;')
                    fns.write("\n\tl2mc_entry = (sai_l2mc_entry_t*) calloc(1,sizeof(sai_l2mc_entry_t));")
                    fns.write("\n\t"+'if(!l2mc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'l2mc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'l2mc_entry->bv_id = bv_id;')
                    fns.write("\n\t"+'l2mc_entry->type = entry_type;')
                    fns.write("\n\t"+'l2mc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'l2mc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(l2mc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

                elif 'saiShellIpmc' == file_name:#need to handle ipmc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'sai_ipmc_entry_t *ipmc_entry = NULL;')
                    fns.write("\n\tipmc_entry = (sai_ipmc_entry_t*) calloc(1,sizeof(sai_ipmc_entry_t));")
                    fns.write("\n\t"+'if(!ipmc_entry) return SAI_STATUS_FAILURE;')
                    fns.write("\n\t"+'ipmc_entry->switch_id = switch_id;')
                    fns.write("\n\t"+'ipmc_entry->vr_id = vr_id;')
                    fns.write("\n\t"+'ipmc_entry->type = entry_type;')
                    fns.write("\n\t"+'ipmc_entry->source.addr_family = (sai_ip_addr_family_t) src_ip_address->addr_family ;')
                    fns.write("\n\t"+'ipmc_entry->destination.addr_family = (sai_ip_addr_family_t) dst_ip_address->addr_family ;')
                    fns.write("\n\t"+'if (ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip4), &(src_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->source.addr.ip6), &(src_ip_address->addr), 16);'+"\n")
                    fns.write("\n\t"+'if (ipmc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip4), &(dst_ip_address->addr), 4);')
                    fns.write("\n\t"+'else')
                    fns.write(" \n\t\t"+'memcpy(&(ipmc_entry->destination.addr.ip6), &(dst_ip_address->addr), 16);')

                if 'switch' not in create['name']:
                    fns.write("\n\t"+'sai_status_t ret = SAI_STATUS_SUCCESS;')
		struct_elemnts = apistruct[create['struct_ptr']]
                if 'switch' in create['name']:
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        fns.write('\n\n\t'+'if((('+z+'*)xpSaiApiTableArr['+ apienums[z]+"]))")
                        fns.write('\n\t{')
                        fns.write('\n\n\t\t'+'((('+z+'*)xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                    else:
                        fns.write("\n\n\t"+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')
                else:
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        fns.write('\n\n\tif((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"])))")
                        fns.write('\n\t{')
                        fns.write('\n\n\t\t'+'ret = ((('+z+'*)(xpSaiApiTableArr['+ apienums[z]+"]))"+'->'+struct_elemnts[create['name']]+'(')
                    else:
                        fns.write("\n\n\t"+'ret = ('+api_struct_ptr+'->'+struct_elemnts[create['name']]+'(')

                string =''

		for x in range(num_of_param):
                    param_list = create['params']

                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][2]+','
                if 'switch' in create['name']:
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        string = string[:-1]+');\n\t}\n}'
                    else:
                        string = string[:-1]+');\n}'
                else:
		    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
		        string = string[:-1]+'));'+'\n\t}'
		    else:
		        string = string[:-1]+'));'+'\n'
                string = string.replace('*','')
                if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                    string = string.replace('_api_t','_api_t*')
                fns.write(string)

                if 'saiShellL2mc' == file_name:#need to free l2mc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'free(l2mc_entry);')
                if 'saiShellIpmc' == file_name:#need to free ipmc seperately as it has a structure in its primary arguments
                    fns.write("\n\t"+'free(ipmc_entry);')
                if 'saiShellRoute' == file_name:
                    fns.write("\n\t"+'free(route_entry);')
                if 'saiShellNeighbor' == file_name:
                    fns.write("\n\t"+'free(neighbor_entry);')

                if 'switch' not in create['name']:
                    fns.write("\n\n\treturn ret;"+"\n}")
                else:
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')) and ('}\n}' not in string):
                        fns.write("\n}")
                #This should be removed after generic implementation
                if 'saiShellFdb' == file_name:
                    string = '\n\nsai_status_t sai_flush_fdb_entries(sai_object_id_t switch_id, uint32_t attr_count, sai_attribute_t *attr_list)\n'
                    string += '{\n'
                    string += '\tsai_status_t ret = SAI_STATUS_SUCCESS;\n'
                    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                        string += '\tif(((sai_fdb_api_t*)(xpSaiApiTableArr[SAI_API_FDB])))\n'
                        string += '\t{\n'
                        string += '\t\tret = (((sai_fdb_api_t*)(xpSaiApiTableArr[SAI_API_FDB]))->flush_fdb_entries(switch_id, attr_count, attr_list));\n'
                        string += '\t}\n'
                    else:
                        string += '\tret = (_xpSaiFdbApi->flush_fdb_entries(switch_id, attr_count, attr_list));\n'
                    string += '\treturn ret;\n'
                    string += '}\n'
                    fns.write(string)

    fns.close()


####################################################################################
#Funtion for generation of Headers 	                                           #
####################################################################################
def gen_hdrs():

#    hdrs = open('sai_shell_hdrs/create_hdrs.h','w')
    filelist = db

    for eachfile in filelist.keys():
	file_name = 'saiShell'+eachfile.replace('sai','').capitalize()+'.h'
	hdrs = open('saiShellCWrappers/include/'+file_name ,'w')
	hdrs.write("\n"+'#ifndef '+'_'+file_name.replace('.','_')+'_')
	hdrs.write("\n"+'#define '+'_'+file_name.replace('.','_')+'_')
	
	hdrs.write("\n"+'#include "'+eachfile+'.h"'+"\n")
	objctlist = filelist[eachfile]

        if file_name == "saiShell.h":
            hdrs.write("\n"+'#include "xpSaiUtil.h"'+"\n\n")
            if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
                hdrs.write("\n"+'#ifdef __cplusplus')
                hdrs.write("\n"+'extern "C" {')
                hdrs.write("\n"+'#endif') 
	    hdrs.write("\n"+'typedef struct shell_mac {'+"\n\tsai_mac_t mac;"+"\n} shell_mac;\n")
	    hdrs.write("\n"+'typedef struct shell_ip6 {'+"\n\tuint8_t addr[16];"+"\n} shell_ip6;\n")
#	    hdrs.write("\n"+'typedef struct _sai_ip_address_t {'+"\n\tint32_t addr_family;"+"\n\tuint8_t addr[16];" +"\n}"+' sai_ip_address_t;\n')
	    hdrs.write("\n"+'typedef struct shell_string {'+"\n\tchar name[30];\n} shell_string;\n")
            hdrs.write("\n"+'sai_status_t xpSaiShellCompressAttributes(uint32_t attr_count, sai_attribute_t *attr_list, uint32_t *final_attr_count, sai_attribute_t *final_attr_list, uint32_t object_attr_count, const xpSaiAttrEntry_t * object_attr_list);'+"\n")

	    hdrs.write("\n\n"+'typedef union field_mask {')
            hdrs.write("\n\tsai_uint8_t u8;")
            hdrs.write("\n\tsai_int8_t s8;")
            hdrs.write("\n\tsai_uint16_t u16;")
            hdrs.write("\n\tsai_int16_t s16;")
            hdrs.write("\n\tsai_uint32_t u32;")
            hdrs.write("\n\tsai_int32_t s32;")
            hdrs.write("\n\tsai_mac_t mac;")
            hdrs.write("\n\tsai_ip4_t ip4;")
            hdrs.write("\n\tsai_ip6_t ip6;")
            hdrs.write("\n\tsai_u8_list_t u8list;\n} field_mask;")

            hdrs.write("\n\n"+'typedef union field_data {')
            hdrs.write("\n\tbool booldata;")
            hdrs.write("\n\tsai_uint8_t u8;")
            hdrs.write("\n\tsai_int8_t s8;")
            hdrs.write("\n\tsai_uint16_t u16;")
            hdrs.write("\n\tsai_int16_t s16;")
            hdrs.write("\n\tsai_uint32_t u32;")
            hdrs.write("\n\tsai_int32_t s32;")
            hdrs.write("\n\tsai_mac_t mac;")
            hdrs.write("\n\tsai_ip4_t ip4;")
            hdrs.write("\n\tsai_ip6_t ip6;")
            hdrs.write("\n\tsai_object_id_t oid;")
            hdrs.write("\n\tsai_object_list_t objlist;")
            hdrs.write("\n\tsai_u8_list_t u8list;\n} field_data;")

            hdrs.write("\n\n"+'typedef struct shell_acl_field_t{'+"\n\tbool enable;\n\tfield_mask mask;\n\tfield_data data;\n}shell_acl_field_t;")

            hdrs.write("\n\n"+'typedef union action_data {')
            hdrs.write("\n\tsai_uint8_t u8;")
            hdrs.write("\n\tsai_int8_t s8;")
            hdrs.write("\n\tsai_uint16_t u16;")
            hdrs.write("\n\tsai_int16_t s16;")
            hdrs.write("\n\tsai_uint32_t u32;")
            hdrs.write("\n\tsai_int32_t s32;")
            hdrs.write("\n\tsai_mac_t mac;")
            hdrs.write("\n\tsai_ip4_t ip4;")
            hdrs.write("\n\tsai_ip6_t ip6;")
            hdrs.write("\n\tsai_object_id_t oid;")
            hdrs.write("\n\tsai_object_list_t objlist;")
            hdrs.write("\n} action_data;")

            hdrs.write("\n\n"+'typedef struct shell_acl_action_t{'+"\n\tbool enable;\n\taction_data parameter;\n} shell_acl_action_t;")

 	else:
	    hdrs.write("\n"+'#include "sai.h"')
	    hdrs.write("\n"+'#include "saiShell.h"')
	    if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
	        hdrs.write("\n"+'#ifdef __cplusplus')
	        hdrs.write("\n"+'extern "C" {')
	        hdrs.write("\n"+'#endif')

	for eachobjct in objctlist.keys():
	    objctmodules = objctlist[eachobjct]

	    param_list = []
	    param_count = []
	    attr_count = 0
	    attr_id_list = []
	    attr_name_list = []
	    fn_list = objctmodules['fn_list']
	    attr_list = objctmodules['attr_list']

	    ####################################################################################
            #Script for generation of "CREATE headers"                                           #
            ####################################################################################
	    if 'CREATE' in fn_list.keys():
		create = fn_list['CREATE']
		default_string = ''
                custom_string = ''
                string = ''
                default_string = default_string+"\n\nsai_object_id_t "+create['name'].replace('_fn','')+'_default('
                custom_string = custom_string+"\n\nsai_object_id_t "+create['name'].replace('_fn','')+'_custom('
		
		num_of_param = create['numparam']
	
		if 'saiShellRoute.h' == file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor.h' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc.h' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc.h' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

		common_string = string
                for m in range(2):
		    string =''
		    string = string+common_string
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
			    if ((m==0)and((attr_proprty['mandatory_on_create'] == 'true') or (attr_proprty['mandatory_on_create'] == 'false' and attr_proprty['valid_for_create']== 'true' and attr_proprty['valid_for_set'] == 'false') or ('saiShellFdb.h' == file_name and attr_proprty['valid_for_create'] == 'true')))or ((m==1)and(attr_proprty['valid_for_create'] == 'true')):
                                if '_list' in attr_proprty['attrib_type'][0]:
				    typ = attr_proprty['attrib_type'][0]
				    string = string+' '+typ+' *'+attr_proprty['attrib_name']+'_ptr'+','

				elif 'sai_ip_address_t' in attr_proprty['attrib_type'][0]:
				    string = string+' sai_ip_address_t *addr_'+attr_proprty['attrib_name']+',' 
			
				elif 'sai_mac_t' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_mac *'+attr_proprty['attrib_name']+','

				elif 'sai_ip6_t' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_ip6 *'+attr_proprty['attrib_name']+','
					
				elif 'char' in attr_proprty['attrib_type'][0]:
				    string = string+' shell_string *'+attr_proprty['attrib_name']+','

				else:
				    if eachobjct == 'acl_entry' and (m==1):
				        if "acl_field" in attr_proprty['attrib_type'][0] or "acl_action" in attr_proprty['attrib_type'][0]:
				            text = attr_proprty['attrib_type'][0].replace('_data','').replace('sai','shell')
				            string = string+' '+text+' *'+attr_proprty['attrib_name']+','
				        else:
				            string = string+' '+attr_proprty['attrib_type'][0]+' '+attr_proprty['attrib_name']+','
				    else:
				        string = string+' '+attr_proprty['attrib_type'][0]+' '+attr_proprty['attrib_name']+','

		    if eachobjct == 'acl_entry' and m ==0 :
                        string = string +' sai_uint32_t priority'+','

		    string = string[:-1]
		    string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
		    if(m==0):
                        default_string = default_string+string
                    elif(m==1):
                        custom_string = custom_string + string

		hdrs.write(default_string+');')
                hdrs.write(custom_string+');')

	    ####################################################################################
            #Script for generation of "SET headers"                                              #
            ####################################################################################
   	    if 'SET' in fn_list.keys():
                create = fn_list['SET']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

                num_of_param = create['numparam']

		if 'saiShellRoute.h' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor.h' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc.h' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc.h' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

                if eachobjct == 'acl_entry':
                    string = string +'uint32_t attr_id, sai_attribute_value_t value, int aclfieldflag, shell_acl_field_t *aclfield, int aclactionflag, shell_acl_action_t *aclaction'
                else:
		    string = string +'uint32_t attr_id, sai_attribute_value_t value, int ipaddr_flag, sai_ip_address_t *ip_addr'

                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int

                hdrs.write(string+');')

	    ####################################################################################
            #Script for generation of GET headers                                              #
            ####################################################################################
	    if 'GET' in fn_list.keys():
		create = fn_list['GET']
                if 'saiShellSwitch.h' == file_name:
                    string = "\n\nsai_object_id_t "+create['name'].replace('_fn','')+'('
                else:
                    string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

                num_of_param = create['numparam']

                if 'saiShellRoute.h' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor.h' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc.h' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc.h' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'

                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

		string = string +' sai_'+eachobjct+'_attr_t attr_id, uint32_t list_count, sai_attribute_t *out_attr'
                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int

		hdrs.write(string+');')
	    ####################################################################################
            #Script for generation of "REMOVE Headers"                                          #
            ####################################################################################

            #TODO Handling list types(array types), mac , ip6 

            if 'REMOVE' in fn_list.keys():
                create = fn_list['REMOVE']
                if 'switch' in create['name']: 
                    string = "\n\nvoid "+create['name'].replace('_fn','')+'('
                else:
                    string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('

		if 'saiShellRoute.h' ==  file_name:#need to handle rout seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' uint32_t ip_addr_family,'+'shell_ip6 *ipaddr, shell_ip6 *mask,'

		elif 'saiShellNeighbor.h' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t rif_id,'
                    string = string+' sai_ip_address_t *ip_address,'
                elif 'saiShellL2mc.h' == file_name:#need to handle l2mc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t bv_id,'
                    string = string+' sai_l2mc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                elif 'saiShellIpmc.h' == file_name:#need to handle ipmc seperately as it has a union in a structure in its primary arguments
                    string = string+'sai_object_id_t switch_id,'
                    string = string+' sai_object_id_t vr_id,'
                    string = string+' sai_ipmc_entry_type_t entry_type,'
                    string = string+' sai_ip_address_t *src_ip_address,'
                    string = string+' sai_ip_address_t *dst_ip_address,'
                else:
                    for x in range(num_of_param):
                        param_list = create['params']

                        if param_list[x][0] == '_In_':
                            string = string+param_list[x][1]+' '
                            string = string+param_list[x][2]+','

                string = string[:-1]
                string = string+');'
                string = string.replace('bool','uint8_t') #TODO python script is not handling bool datatype so converting it to int

                hdrs.write(string)

            if 'GET_STATS' in fn_list.keys():
                create = fn_list['GET_STATS']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']

                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        string = string+param_list[x][2]+','
                string = string + "uint64_t *counters);"
                string = string.replace('bool','uint8_t') #TODO python script is not handling bool datatype so converting it to int

                hdrs.write(string)

            if 'GET_STATS_EXT' in fn_list.keys():
                create = fn_list['GET_STATS_EXT']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']

                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        string = string+param_list[x][2]+','
                string = string + "uint64_t *counters);"
                string = string.replace('bool','uint8_t') #TODO python script is not handling bool datatype so converting it to int

                hdrs.write(string)

            if 'CLEAR_STATS' in fn_list.keys():
                create = fn_list['CLEAR_STATS']
                string = "\n\nsai_status_t "+create['name'].replace('_fn','')+'('
                num_of_param = create['numparam']

                for x in range(num_of_param):
                    param_list = create['params']
                    if param_list[x][0] == '_In_':
                        string = string+param_list[x][1]+' '
                        if x == (num_of_param - 1):
                            string = string+param_list[x][2]+');'
                        else:
                            string = string+param_list[x][2]+','
                string = string.replace('bool','uint8_t') #TODO python is not handling bool datatype so converting it to int
                hdrs.write(string)
                if 'saiShellPort.h' == file_name:
                    hdrs.write("\n\nsai_status_t sai_clear_port_all_stats(sai_object_id_t port_id);")
        if 'saiShellFdb.h' == file_name:#need to handle neighbor seperately as it has a structure in its primary arguments
            string = '\n\nsai_status_t sai_flush_fdb_entries(sai_object_id_t switch_id, uint32_t attr_count, sai_attribute_t *attr_list);\n'
            hdrs.write(string)
        if(len(sys.argv) == 2 and (sys.argv[1] == '-k' or sys.argv[1] == '--kernel')):
            hdrs.write("\n"+'#ifdef __cplusplus')
            hdrs.write("\n"+'}')
            hdrs.write("\n"+'#endif')
        hdrs.write("\n"+'#endif')
    hdrs.close()            

if __name__ == "__main__":

    if len(sys.argv) > 2:
        print "Invalid option"
        sys.exit(0)
# Add kernel dependacy code to compile ksaiShell_py
    elif len(sys.argv) == 2:
        if(sys.argv[1] == '-k' or sys.argv[1] == '--kernel'):
            fp = open('../genScripts/saiShellCWrappers/include/shellprinting.h','r')

            count = 0 
            readList = []
            line = ''

            for line in fp: 
                readList.append(line)

            fp.close()

            if '#ifdef __cplusplus\n' not in readList:
                fp = open("../genScripts/saiShellCWrappers/include/shellprinting.h","w+")
                for line in readList:
                    if count == 0:
                        if 'sai_status_t' in line or 'void' in line:
                            count = 1 
                            fp.write("#ifdef __cplusplus\n")
                            fp.write('extern "C" {\n')
                            fp.write("#endif\n")
                    fp.write(line)
                fp.write("#ifdef __cplusplus\n")
                fp.write("}\n")
                fp.write("#endif\n")
                fp.close()


# Corner case: if user run preprocessing.sh with --kernel mode and then again run preprocessing.sh without --kernel mode then remove kernel specific code
    elif len(sys.argv) == 1:
        ignoreList = ['#ifdef __cplusplus\n','extern "C" {\n','#endif\n','#ifdef __cplusplus\n','}\n','#endif\n']
        fp = open('../genScripts/saiShellCWrappers/include/shellprinting.h','r')
        count = 0 
        readList = []
        line = ''

        for line in fp: 
            readList.append(line)

        fp.close()

        fp = open("../genScripts/saiShellCWrappers/include/shellprinting.h","w")
        for line in readList:
            if line not in ignoreList:
                fp.write(line)
        fp.close()

    else:
        print "Invalid option"
        sys.exit(0)


    gen_fns()
    gen_hdrs()
    processManualStrings()



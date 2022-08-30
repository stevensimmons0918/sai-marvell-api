
import sys
import os
import cPickle
import re
from xml.etree import ElementTree

file_name = '../../xpSai/sai/xml/all.xml'
full_file = os.path.join('',file_name)

dom = ElementTree.parse(full_file)

root= dom.getroot()

# Defined strings for functions
keystr = 'sai_status_t'
create = 'sai_create_'
remove = 'sai_remove_'
saiset = 'sai_set_'
saiclear = 'sai_clear_'
saiget = 'sai_get_'
saistats = '_stats_fn'
saistats_ext = '_stats_ext_fn'
apikey = '_api_t'

# Declare dictionaries 
file_list ={}
list_struct_dict ={}
union_membrs = {}
api_structures ={}
api_enums = {}

# Define Strings for attributes
keystr = "_attr_t"

#Each attribute-id has a flag associated with it which defines the scope of that attribute.If the flag is READ-ONLY then the attribute can only be read,If CREATE-AND-SET then the attribute can be used while creating an object and also can be modified later and so on.. SAI 1.0 has six types of flags which are stated below.
#TODO UPDATE Required if new flags introduced
readonly = ['READ-ONLY', 'READ_ONLY']
readwrite =['READ-WRITE', 'READ_WRITE']
createset = ['CREATE_AND_SET', 'CREATE-AND-SET']
mand_on_create_only =['MANDATORY_ON_CREATE|CREATE_ONLY', 'MANDATORY_ON_CREATE | CREATE_ONLY']
mand_on_create_and_set = ['MANDATORY_ON_CREATE | CREATE_AND_SET','MANDATORY_ON_CREATE|CREATE_AND_SET']
createonly = ['CREATE_ONLY','CREATE-ONLY']


# This funtion parses the xml file to find sai.h file and finds the sai_api_t enum which contains an ENUM for each API Structure defined in the header file
# Calling the following funtion will generate a dict(api_enums) with keys as api structure names and values as ENUMS eg: key = "sai_vlan_api_t" value="SAI_API_VLAN"
def getSaiApiEnumFromXml():
    
    for child in root:
        if child.attrib['kind'] == 'file':
	    if 'sai.h' in child.find('compoundname').text:
		sections = child.findall('sectiondef')

	        for ab in sections:
                    if ab.attrib['kind'] == 'enum':
			for member in ab:
			    if '_sai_api_t' in member.find('name').text:
 				
				enums = member.findall('enumvalue')
				for enum in enums[0:-1]:
				    det_description = enum.find('detaileddescription')
				    api_set = det_description.find('para').text.strip()
				    if api_set == 'sai_host_interface_api_t': #In the SAI repo there is an error for sai_host_interface_api_t so have to handle it.
					api_set = 'sai_hostif_api_t'
                                    api_set = api_set.replace(' (experimental)','')
				    api_enums[api_set] = enum.find('name').text.strip()



# This funtions generates a dictionary containing the definition of all the api structures which are defined at the end of each sai-header file,these API structure contain instances of all function pointers defined in that file as its elements.
def getApiStructFromXml():
    for child in root:
        if child.attrib['kind'] == 'struct'  and apikey in (child.find('compoundname').text):
	    api_struct = child.find('compoundname').text[1:]
	    sections = child.findall('sectiondef')

	    struct_elements ={}
	    for ab in sections:
		for member in ab:
		    definition = member.find('definition').text
		    func_ptr_type = definition.split(' ')[0]
		    func_ptr      = member.find('name').text
		    struct_elements[func_ptr_type] = func_ptr
	    api_structures[api_struct] = struct_elements



# This function parses the xml file to find all the CREATE, SET, GET, REMOVE Apis along with their arguments and returntypes and writes into the database 
def getFuncDSFromXml():

    noFunction =0
    for child in root:
        if child.attrib['kind'] == 'file':
            # File name is present in element compoundname.text
            filename = (child.find('compoundname').text).replace('.h','')

            sections = child.findall('sectiondef')

            for ab in sections:
                if ab.attrib['kind'] == 'typedef':
                    for members in ab:
                        tempstring =  members.find('name').text
                        if apikey in tempstring:
                            #objectapi = tempstring.replace('_',' ').title().replace(' ','')
                            #api_struct_ptr = '_xp'+objectapi[:-1]
                            api_struct_ptr = tempstring
                            break

            for eachSection in sections:

                # This loop will searh the sections(xml sections) to find the sections with section.kind = typedef and section.member.type = sai_status_t(*
                if eachSection.attrib['kind'] == 'typedef':
                    allFuncList={}
                    for member in eachSection:
                        if (member.find('type').text == 'sai_status_t(*') or (member.find('type').text == 'void(*'):
                            objctname = ""
                            #for funcType in (create,remove,saiset,saiget):
                            #allFuncList = {}
                            funcType = ""
                            if member.find('name').text.startswith(create):
                                funcType = 'CREATE'
                                objctname = member.find('name').text.replace(create,'').replace('_fn','')
                                noFunction = noFunction + 1

                            elif member.find('name').text.startswith(remove):
                                #continue
                                funcType = 'REMOVE'
                                objctname = member.find('name').text.replace(remove,'').replace('_fn','')

                            elif member.find('name').text.startswith(saiset) and ('attr' in  member.find('name').text):
                                #continue
                                funcType = 'SET'
                                objctname = member.find('name').text.replace(saiset,'').replace('_attribute_fn','').replace('_attr_fn','')

                            elif member.find('name').text.startswith(saiget) and ('attr' in  member.find('name').text):
                                #continue
                                funcType = 'GET'
                                objctname = member.find('name').text.replace(saiget,'').replace('_attribute_fn','').replace('_attr_fn','')

                            elif (member.find('name').text.startswith(saiget) and saistats in  member.find('name').text):
                                #continue
                                funcType = 'GET_STATS'
                                objctname = member.find('name').text.replace(saiget,'').replace(saistats,'')
                            elif (member.find('name').text.startswith(saiget) and saistats_ext in  member.find('name').text):
                                #continue
                                funcType = 'GET_STATS_EXT'
                                objctname = member.find('name').text.replace(saiget,'').replace(saistats_ext,'')
                            elif (member.find('name').text.startswith(saiclear) and saistats in  member.find('name').text):
                                #continue
                                funcType = 'CLEAR_STATS'
                                objctname = member.find('name').text.replace(saiclear,'').replace(saistats,'')
                            else:
                                #continue
                                funcType = 'TODO'
                                objctname = member.find('name').text.replace('','').replace('_fn','')
                            #print "******************"
                            funcList = {}
                            apiName = member.find('name').text
                            apiArgString = member.find('argsstring').text.replace(")(","").replace(")","")
                            #neglecting last two params as they are common attr_count and *attr_list
                            noParams=0
                            noTotalParams = len(apiArgString) - len(apiArgString.replace(",","")) + 1
                            if funcType == "CREATE" or funcType =="GET":
                                noParams = noTotalParams - 2
                            elif funcType == "SET":
                                noParams = noTotalParams - 1
                            elif funcType == "REMOVE":
                                noParams = noTotalParams
                            elif funcType == "GET_STATS":
                                noParams = noTotalParams - 1
                            elif funcType == "GET_STATS_EXT":
                                noParams = noTotalParams - 1
                            elif funcType == "CLEAR_STATS":
                                noParams = noTotalParams
                            else:
                                noParams = noTotalParams
                            print apiName
                            params = apiArgString.split(",")[0:noParams]
                            paramList = []
                            for eachParam in params:
                                eachParamList = []
                                eachParam = eachParam.strip()
                                eachParamLen = len(eachParam.split(" "))
                                paramDir = eachParam.split(" ")[0]
                                paramName = eachParam.split(" ")[-1]
                                #type of parameter is everything in between then param direction and param name
                                paramType = eachParam.split(" ")[1:eachParamLen-1]
                                paramType = " ".join(paramType)
                                eachParamList=[paramDir, paramType, paramName]
                                paramList.append(eachParamList)
                            funcList['name'] = apiName
                            funcList['numparam'] = noParams
                            funcList['params'] = paramList
                            funcList['struct_ptr'] = api_struct_ptr 
                            #allFuncList[funcType] = funcList

                            x =file_list[filename]

                            #We get the object-name from the API but Few Objects have naming discrepancies so hnadling those cases
                            if objctname == 'route':
                                objctname = 'route_entry'
                            elif objctname == 'samplepacket_session':
                                objctname = 'samplepacket'
                            elif objctname == 'neighbor':
                                objctname = 'neighbor_entry'
                            try:
                                y = x[objctname]
                                y['fn_list'][funcType] = funcList
                                #print noFunction
                            except KeyError, e:
                                #TODO - ADD ALL THAT DOES NOT MATCH OBJECT NAME UNDER A DUMMY ATTRIBUTE
                                print 'I got a KeyError - reason "%s"' % str(e)
                                print funcList

def getAttrDSFromxml():
    # Define Strings for attributes
    keystr = ""
    typeset = "NO"
    flagset = "NO"
    nameset = "NO"
    
    attr_proprty = {}
    objct_list = {}
    objct_attr_list = {}
    objct_modules ={}

    for child in root:
        # Search for child of kind file in the xml file
        if child.attrib['kind'] == 'file':
        # File name is present in element compoundname.text
            filename = (child.find('compoundname').text).replace('.h','')
    
            # Each compound has multiple sections
            sections = child.findall('sectiondef')
    
            # Search for sections of kind enum
            for x in sections:
                if x.attrib['kind'] == 'enum':
                    # Section of kind enum has all the defined enums as its members
                    for y in x:
                        keystr = ""
                        # Check if enum y  is that of attributes 
                        string = y.find('name').text
                        # keystr = "_attr_t" if keystr present in string then the enum is defined for attribute_list 
                        if "_attr_t" in string:
                            keystr = "_attr_t"
                        if "_stat_t" in string:
                            keystr = "_stat_t"
                        if keystr != "":
                            # Framing the object name from enum name
                            objctname = string.replace('_sai_','').replace(keystr,'').strip()
                            for z in y:
                                # Element y has the all attributes as its members 
                                # Check if member is of type 'enumvalue'
                                if z.tag == 'enumvalue':
                                    attr_id = z.find('name').text
                                    attr_proprty['attr_id']=attr_id
                                    detail = z.find('detaileddescription') # Element 'detaileddescription' has the properties of the attributes as its children
                                    flagset = "NO"
                                    # Iterate through the children of detaileddescription to know the property of the attrributes
                                    for ab in detail:
            
                                        for ppty in ab:
                                            # Check if the child is defined "Flag" property of the attribute
                                            if ((ppty.tag == 'simplesect') and (ppty.find('title').text) == "Flags:"):
                                                flag_string = ''
                                                flag_string = ppty.find('para').text
            
                                                if flagset == "NO":
                                                    for a in readonly:
                                                        if a in flag_string  :
                                                            attr_proprty['mandatory_on_create'] ='false'
                                                            attr_proprty['valid_for_create'] ='false'
                                                            attr_proprty['valid_for_set'] ='false'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break
                                                if flagset == "NO":
                                                    for a in  readwrite :
                                                        if a in flag_string:
                                                            attr_proprty['mandatory_on_create'] ='false'
                                                            attr_proprty['valid_for_create'] ='false'
                                                            attr_proprty['valid_for_set'] ='true'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break
                                                if flagset == "NO":
                                                    for a in mand_on_create_only :
                                                        if a in flag_string:
                                                            attr_proprty['mandatory_on_create'] ='true'
                                                            attr_proprty['valid_for_create'] ='true'
                                                            attr_proprty['valid_for_set'] ='false'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break
                                                if flagset == "NO":
                                                    for a in mand_on_create_and_set : 
                                                        if a in flag_string:
                                                            attr_proprty['mandatory_on_create'] ='true'
                                                            attr_proprty['valid_for_create'] ='true'
                                                            attr_proprty['valid_for_set'] ='true'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break
                                                if flagset == "NO":
                                                    for a in createset :
                                                        if a in flag_string :
                                                            attr_proprty['mandatory_on_create'] ='false'
                                                            attr_proprty['valid_for_create'] ='true'
                                                            attr_proprty['valid_for_set'] ='true'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break 
                                                if flagset == "NO":
                                                    for a in createonly:
                                                        if a in flag_string :
                                                            attr_proprty['mandatory_on_create'] ='false'
                                                            attr_proprty['valid_for_create'] ='true'
                                                            attr_proprty['valid_for_set'] ='false'
                                                            attr_proprty['valid_for_get'] ='true'
                                                            flagset = "YES"
                                                            break
                                            else:
                                                continue
                                    if flagset =="NO":
                                        attr_proprty['mandatory_on_create'] ='false'
                                        attr_proprty['valid_for_create'] ='false'
                                        attr_proprty['valid_for_set'] ='false'
                                        attr_proprty['valid_for_get'] ='false'
                                    # Build the attribute name from attr_id
                                    temp_name = attr_id.lower().replace('sai_','').replace('_attr_','')
                                    if temp_name.startswith(objctname):
                                        temp_name = temp_name[len(objctname):].strip()
                                    if temp_name[0].isdigit():
                                        temp_name = "_" + temp_name
                                    attr_proprty['attrib_name'] = temp_name
                                    nameset = "YES"
            
                                    # Parse to find the attribute type

                                    detail = z.find('detaileddescription')
                                    ignore_attr = "NO"
                                    for ab in detail:
                                        for ppty in ab:
                                            if ((ppty.tag == 'simplesect') and (ppty.find('title').text) == "Value Type:"):
                                                type_string = ppty.find('para').text
                                                type_list = type_string.replace('@@type','').strip().split(' ')
                                                attr_proprty['attrib_type'] = type_list
                                                #attr_proprty['attrib_type'] = type_string.replace('@@type','').strip()
                                                typeset = "YES"
                                            if ((ppty.tag == 'simplesect') and (ppty.find('title').text) == "Ignored:"):
                                                print("Ignore attribute: " + attr_id + "\n")
                                                ignore_attr = "YES"
                                    if typeset == "NO":
                                        attr_proprty['attrib_type'] = ['undetermined']
                                    typeset = "NO"
                                    if ((not attr_id.endswith('START')) and not (attr_id.endswith('END')) and (ignore_attr == 'NO')):#Should Discard the START and END 
                                        objct_attr_list[attr_id] = attr_proprty
                                    attr_proprty = {}
        
                            objct_modules['attr_list'] = objct_attr_list
                            objct_modules['fn_list'] = {}
                            if objctname in objct_list:
                                temp = objct_list[objctname]
                                temp['stats_attr_list'] = objct_attr_list
                                objct_list[objctname] = temp
                            else:
                                objct_list[objctname] = objct_modules
                            #objct_list[objctname]= [objct_attr_list]
                            objct_attr_list = {}
                            objct_modules = {}
            file_list[filename]=objct_list
            objct_list = {}

def genListDSfromXml():

	key_str = '_list_t'
	#list_struct_dict ={}

	for child in root:
	    if child.attrib['kind'] == 'struct':
		if key_str in (child.find('compoundname').text):
		    
		    struct_name = (child.find('compoundname').text)[1:]
		    section = child.find('sectiondef')
		    members = section.findall('memberdef')
		    var_dict = {}
		    for member in members:
		        if member.attrib['kind'] == 'variable':
		            var = ['','']
			    var[0] = (member.find('definition').text).split(' ')[0]
			    #print var[0]
			    var[1] = (member.find('name').text)
		    	    var_dict[var[1]] = var
			    #print var
		    list_struct_dict[struct_name] = var_dict


def genUnionDSFromXml():
	key_str = 'sai_attribute_value_t'
	
	for child in root:
	    if child.attrib['kind'] == 'union':
		if key_str in (child.find('compoundname').text):
		
		    union_name  = child.find('compoundname').text
		    section = child.find('sectiondef')
		    members = section.findall('memberdef')
		    
		    for member in members:
			memberdef = member.find('definition').text
			union_membrs[memberdef.split(' ')[0]] = member.find('name').text

def printUnionDS():
	sys.stdout=open('union_db','w')
	for x in union_membrs:
		print x
		y = union_membrs[x]
		print "\n\t",y,"\n"
			

def printListDS():
	sys.stdout=open('liststruct_db','w')
	for x in  list_struct_dict:
		print x
		y = list_struct_dict[x]
	
		for z in y:
		    a = y[z]
		    print a


def printDS():

	sys.stdout=open('attr_db','w')
	

	for key in file_list.keys():
    	    a = file_list[key]
    	    print key
    	    for subkeys in a.keys() :
        	print ' ',subkeys
        	b = a[subkeys]
        	for vals in b.keys():
            		print '             ',vals
			b1 = b[vals]
			for vals1 in b1.keys():
			    print "\t\t\t",vals1
			    b2 = b1[vals1]
			    #print b2,"KKKKKKK"
			    for vals2 in b2.keys():
				print "\t\t\t\t",vals2
				b3 = b2[vals2]
				print "\t\t\t\t\t",b3
				'''for vals3 in b3.keys():
					print "\t\t\t\t\t",vals3'''



def printSaiApiEnums():
    sys.stdout=open('api_enum','w')

    for key in api_enums.keys():
	print key
	c =  api_enums[key]
	print c 
# The controller of the sai shell
if __name__ == "__main__":

    print "\n\n"
    getAttrDSFromxml()
    print "--------------Getting Attribute Datastructures from xml Done--------------"
    getFuncDSFromXml()
    print "--------------Getting Function Datastructures from xml Done--------------"
    genListDSfromXml()
    print "--------------Getting List Datastructures from xml Done--------------"
    genUnionDSFromXml()
    print "--------------Getting Union Datastructures from xml Done--------------"
    getApiStructFromXml()
    print "--------------Getting Api structures from xml Done--------------"
    getSaiApiEnumFromXml()
    print "--------------Getting Sai Api Enum Datastructures from xml Done--------------"
    #printDS()
    #printListDS()
    #printUnionDS()
    #printSaiApiEnums()
    #print file_list
    #Creating file_list library to be used in other files
    f = open("db_file.txt","w")
    cPickle.dump(file_list,f)
    f.close()

    f = open("list_struct_db.txt","w")
    cPickle.dump(list_struct_dict,f)
    f.close()

    f = open("union_db.txt","w")
    cPickle.dump(union_membrs,f)
    f.close()

    f = open("api_structures.txt","w")
    cPickle.dump(api_structures,f)
    f.close()

    f = open("api_enums.txt","w")
    cPickle.dump(api_enums,f)
    f.close()

    #f = open('db_file','w')
    #f.write(file_list)
    #print file_list

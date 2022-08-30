#define __STDC_FORMAT_MACROS 1

#include "shellprinting.h"

void print_attribute(char *attr_type , sai_attribute_value_t value, sai_attribute_t *outAttr)
{
    uint32_t j=0;
    uint32_t addr = 0;
    sai_ip6_t ip6addr ;

    if( strcmp(attr_type , "bool") == 0 )
    {
        printf("%d\n",  value.booldata);
        if(outAttr)
        {
            outAttr->value.booldata = value.booldata;
        }
    }
    else if(strcmp(attr_type , "char") == 0)
    {
        for(j=0; value.chardata[j] != '\0' ;j++)
        {
            printf("%c",value.chardata[j]);
            if(outAttr)
            {
                outAttr->value.chardata[j] = value.chardata[j];
            }
        }
        printf("\n");		
    }
    else if(strcmp(attr_type, "sai_uint8_t") ==0)
    {
        printf("%d\n", (int32_t) value.u8);
        if(outAttr)
        {
            outAttr->value.u8 = value.u8;
        }
    }
    else if(strcmp(attr_type, "sai_int8_t") ==0)
    {
        printf("%d\n", (int32_t) value.s8);
        if(outAttr)
        {
            outAttr->value.s8 = value.s8;
        }
    }
    else if(strcmp(attr_type, "sai_uint16_t") ==0)
    {
        printf("%d\n", (int32_t) value.u16);
        if(outAttr)
        {
            outAttr->value.u16 = value.u16;
        }
    }
    else if(strcmp(attr_type, "sai_int16_t") ==0)
    {
        printf("%d \n", (int32_t) value.s16);
        if(outAttr)
        {
            outAttr->value.s16 = value.s16;
        }
    }
    else if(strcmp(attr_type, "sai_uint32_t") ==0)
    {
        printf("%d \n", (int32_t) value.u32);
        if(outAttr)
        {
            outAttr->value.u32 = value.u32;
        }
    }
    else if(strcmp(attr_type, "sai_int32_t") ==0)
    {
        printf("%d\n", (int32_t) value.s32);
        if(outAttr)
        {
            outAttr->value.s32 = value.s32;
        }
    }
    else if(strcmp(attr_type , "sai_uint64_t") ==0)
    {
        printf("%" PRIu64 "\n", value.u64);
        if(outAttr)
        {
            outAttr->value.u64 = value.u64;
        }
    }
    else if(strcmp(attr_type , "sai_int64_t") ==0)
    {
        printf("%" PRId64 "\n", value.s64);
        if(outAttr)
        {
            outAttr->value.s64 = value.s64;
        }
    }	
    else if(strcmp(attr_type , "sai_pointer_t")==0)
    {
        printf("%" PRIu64 "\n", (uint64_t)value.ptr);
        if(outAttr)
        {
            outAttr->value.ptr = value.ptr;
        }
    }
    else if(strcmp(attr_type , "sai_mac_t")==0)
    {
        for (j=0;j<=5;j++)
        {
            printf("%x:", value.mac[j]);
        }
        printf("\n");
        if(outAttr)
        {
            memcpy(outAttr->value.mac, value.mac, sizeof(sai_mac_t));
        }
    }
    else if(strcmp(attr_type , "sai_ip4_t")==0)
    {
        uint32_t len = sizeof(sai_ip4_t);
        addr = value.ip4;
        for(j = 0; j < len-1; j++){
            printf("%d.", ((uint8_t*)&addr)[j]);
        }
        printf("%d\n", ((uint8_t*)&addr)[j]);
        if(outAttr)
        {
            outAttr->value.ip4 = value.ip4;
        }
    }
    else if(strcmp(attr_type , "sai_ip6_t")==0)
    {
        for (j=0;j<15;j++)
            printf("%x:", value.ip6[j]);
        printf("%x",value.ip6[j]);
        if(outAttr)
        {
            memcpy(outAttr->value.ip6, value.ip6, sizeof(sai_ip6_t));
        }
    }
    else if(strcmp(attr_type , "sai_ip_address_t") == 0)
    {
        if (value.ipaddr.addr_family == 0){
            uint32_t len = sizeof(sai_ip4_t);
            memcpy(&addr ,&(value.ipaddr.addr), 4);
            for(j = 0; j < len-1; j++){
                printf("%d.", ((uint8_t*)&addr)[j]);
            }
            printf("%d\n", ((uint8_t*)&addr)[j]);
        }
        else {
            memcpy(ip6addr ,&(value.ipaddr.addr), 16);
            for (j=0;j<15;j++)
                printf("%x:", ip6addr[j]);
            printf("%x\n",ip6addr[j]);
        }
        if(outAttr)
        {
            memcpy(&outAttr->value.ipaddr, &value.ipaddr, sizeof(sai_ip_address_t));
        }
    }

    else if(strcmp(attr_type , "sai_object_id_t") == 0)
    {
        printf("%" PRIu64 "\n", (uint64_t) value.oid);
        if(outAttr)
        {
            outAttr->value.oid = value.oid;
        }
    }
    else if(strcmp(attr_type , "sai_object_list_t") ==0)
    {
        for (j=0;j< value.objlist.count ; j++)
        {
            printf("%" PRIu64 " ", (uint64_t) value.objlist.list[j]);
        }
        printf("\n");		
        if(outAttr)
        {
            outAttr->value.objlist.count = value.objlist.count;
            if(outAttr->value.objlist.list)
            {
                memcpy(outAttr->value.objlist.list, value.objlist.list, 
                        (sizeof(sai_object_id_t) * (value.objlist.count)));
            }
        }
    }

    else if(strcmp(attr_type , "sai_acl_resource_list_t") ==0)
    {
        for (j=0;j< value.aclresource.count ; j++)
        {
            printf("%d  ", value.aclresource.list[j].avail_num);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.aclresource.count = value.aclresource.count;
            if(outAttr->value.aclresource.list)
            {
                memcpy(outAttr->value.aclresource.list, value.aclresource.list, 
                        (sizeof(sai_acl_resource_t) * (value.aclresource.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_u8_list_t") ==0)
    {
        for (j=0;j< value.u8list.count ; j++)
        {
            printf("%d  ", value.u8list.list[j]);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.u8list.count = value.u8list.count;
            if(outAttr->value.u8list.list)
            {
                memcpy(outAttr->value.u8list.list, value.u8list.list,
                        (sizeof(uint8_t) * (value.u8list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_s8_list_t") ==0)
    {
        for (j=0;j< value.s8list.count ; j++)
        {
            printf("%d  ", value.s8list.list[j]);
        }
        printf("\n");	
        if(outAttr)
        {
            outAttr->value.s8list.count = value.s8list.count;
            if(outAttr->value.s8list.list)
            {
                memcpy(outAttr->value.s8list.list, value.s8list.list,
                        (sizeof(int8_t) * (value.s8list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_u16_list_t") ==0)
    {
        for (j=0;j< value.u16list.count ; j++)
        {
            printf("%d  ", value.u16list.list[j]);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.u16list.count = value.u16list.count;
            if(outAttr->value.u16list.list)
            {
                memcpy(outAttr->value.u16list.list, value.u16list.list,
                        (sizeof(uint16_t) * (value.u16list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_s16_list_t") ==0){
        for (j=0;j< value.s16list.count ; j++)
        {
            printf("%d  ", value.s16list.list[j]);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.s16list.count = value.s16list.count;
            if(outAttr->value.s16list.list)
            {
                memcpy(outAttr->value.s16list.list, value.s16list.list,
                        (sizeof(int16_t) * (value.s16list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_u32_list_t") ==0)
    {
        for (j=0;j< value.u32list.count ; j++)
        {
            printf("%d  ", value.u32list.list[j]);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.u32list.count = value.u32list.count;
            if(outAttr->value.u32list.list)
            {
                memcpy(outAttr->value.u32list.list, value.u32list.list,
                        (sizeof(uint32_t) * (value.u32list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_s32_list_t") ==0)
    {
        for (j=0;j< value.s32list.count ; j++)
        {
            printf("%d  ", value.s32list.list[j]);
        }
        printf("\n");
        if(outAttr)
        {
            outAttr->value.s32list.count = value.s32list.count;
            if(outAttr->value.s32list.list)
            {
                memcpy(outAttr->value.s32list.list, value.s32list.list,
                        (sizeof(int32_t) * (value.s32list.count)));
            }
        }
    }
    else if(strcmp(attr_type , "sai_u32_range_t") ==0)
    {
        printf("%d ",value.u32range.min);
        printf("%d\n",value.u32range.max);
        if(outAttr)
        {
            outAttr->value.u32range.min = value.u32range.min;
            outAttr->value.u32range.max = value.u32range.max;
        }
    }

    else if(strcmp(attr_type , "sai_s32_range_t") ==0)
    {
        printf("%d ",value.s32range.min);
        printf("%d \n",value.s32range.max);
        if(outAttr)
        {
            outAttr->value.s32range.min = value.s32range.min;
            outAttr->value.s32range.max = value.s32range.max;
        }
    }
    else if(strcmp(attr_type , "sai_vlan_list_t") ==0)
    {
        for (j=0;j< value.vlanlist.count ; j++)
        {
            printf("%d  ", (uint32_t) value.vlanlist.list[j]);
        }

        printf("\n");
        if(outAttr)
        {
            outAttr->value.vlanlist.count = value.vlanlist.count;
            if(outAttr->value.vlanlist.list)
            {
                memcpy(outAttr->value.vlanlist.list, value.vlanlist.list,
                        (sizeof(sai_vlan_id_t) * (value.vlanlist.count)));
            }
        }
    }
    else if(strcmp(attr_type, "sai_qos_map_list_t") == 0)
    {
        printf("-------------------------------------------------------------------\n");
        printf("|%16s|%5s|%5s|%5s|%5s|%5s|%5s|%12s|\n","key/value","tc","dscp","dot1p","prio","pg","color","queue_index");
        for(j=0 ; j< value.qosmap.count ; j++)
        {
            printf("-------------------------------------------------------------------\n");
            printf("|%10s#%-5d|%5d|%5d|%5d|%5d|%5d|%5d|%12d|\n", "key", j, value.qosmap.list[j].key.tc, value.qosmap.list[j].key.dscp, value.qosmap.list[j].key.dot1p, value.qosmap.list[j].key.prio, value.qosmap.list[j].key.pg, value.qosmap.list[j].key.color, value.qosmap.list[j].key.queue_index);
            printf("|%10s#%-5d|%5d|%5d|%5d|%5d|%5d|%5d|%12d|\n", "value", j, value.qosmap.list[j].value.tc, value.qosmap.list[j].value.dscp, value.qosmap.list[j].value.dot1p, value.qosmap.list[j].value.prio, value.qosmap.list[j].value.pg, value.qosmap.list[j].value.color, value.qosmap.list[j].value.queue_index);
        }
        printf("-------------------------------------------------------------------\n");

        if(outAttr)
        {
            outAttr->value.qosmap.count = value.qosmap.count;
            if(outAttr->value.qosmap.list)
            {
                memcpy(outAttr->value.qosmap.list, value.qosmap.list,
                        (sizeof(sai_qos_map_t) * (value.qosmap.count)));
            }
        }
    }
    else if(strcmp(attr_type, "sai_qos_map_type_t") == 0)
    {
        printf("qos map type : %d \n", value.s32);
        if(outAttr)
        {
            outAttr->value.s32 = value.s32;
        }
    }
    else
    {
        printf("%d \n",value.s32);
        if(outAttr)
        {
            outAttr->value.s32 = value.s32;
        }
    }		
}

	
void acl_print_attribute(char* maintype,char* attr_type, sai_attribute_value_t value, sai_attribute_t *outAttr)
{
    uint32_t j=0;
    uint32_t addr = 0;

    if( strcmp(maintype, "sai_acl_field_data_t") == 0 )
    {	
        if(outAttr)
        {
            memcpy(&(outAttr->value.aclfield), &(value.aclfield), 
                    (sizeof(sai_acl_field_data_t)));
        }

        if(strcmp(attr_type, "sai_uint8_t") ==0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.u8); 
            printf("data = %d\n", (int) value.aclfield.data.u8);
        }

        else if(strcmp(attr_type, "sai_int8_t") == 0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.s8);
            printf("data = %d\n", (int) value.aclfield.data.s8);
        }

        else if(strcmp(attr_type, "sai_uint16_t") ==0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.u16); 
            printf("data = %d\n", (int) value.aclfield.data.u16);
        }

        else if(strcmp(attr_type, "sai_int16_t") == 0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.s16);
            printf("data = %d\n", (int) value.aclfield.data.s16);
        }
        else if(strcmp(attr_type, "sai_uint32_t") ==0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.u32); 
            printf("data = %d\n", (int) value.aclfield.data.u32);
        }

        else if(strcmp(attr_type, "sai_int32_t") == 0)
        {   printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = %d\n",(int) value.aclfield.mask.s32);
            printf("data = %d\n", (int) value.aclfield.data.s32);
        }
        else if(strcmp(attr_type , "sai_mac_t")==0)
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask =");
            for (j=0;j<5;j++)
                printf("%x:", value.aclfield.mask.mac[j]);
            printf("%x\n",value.aclfield.mask.mac[j]);
            printf("data ="); 
            for (j=0;j<5;j++)
                printf("%x:", value.aclfield.data.mac[j]);
            printf("%x\n",value.aclfield.data.mac[j]);

        }

        else if(strcmp(attr_type , "sai_ip4_t")==0)
        {
            uint32_t len = sizeof(sai_ip4_t);
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = ");
            addr = value.aclfield.mask.ip4;
            for(j = 0; j < len-1; j++)
            {
                printf("%d.", ((uint8_t*)&addr)[j]);
            }
            printf("%d\n", ((uint8_t*)&addr)[j]);

            printf("data = ");
            addr = value.aclfield.data.ip4;
            for (j = 0; j < len-1; j++)
            {
                printf("%d.", ((uint8_t*)&addr)[j]);
            }
            printf("%d\n", ((uint8_t*)&addr)[j]);
        }

        else if(strcmp(attr_type , "sai_ip6_t")==0)
        {

            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask = ");
            for (j=0;j<15;j++)
                printf("%x:", value.aclfield.mask.ip6[j]);
            printf("%x\n",value.aclfield.mask.ip6[j]);
            printf("data = ");
            for (j=0;j<15;j++)
                printf("%x:", value.aclfield.data.ip6[j]);
            printf("%x\n",value.aclfield.data.ip6[j]);
        }

        else if(strcmp(attr_type , "sai_object_id_t") == 0)
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("data = ");
            printf("%" PRIu64 "\n", (uint64_t) value.aclfield.data.oid);
        }

        else if(strcmp(attr_type , "bool") == 0)
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("data = ");
            printf("%d\n", (int) value.aclfield.data.booldata);
        }
        else if(strcmp(attr_type , "sai_object_list_t") ==0)
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            for (j=0;j< value.aclfield.data.objlist.count ; j++)
            {
                printf("%" PRIu64 " ", (uint64_t) value.aclfield.data.objlist.list[j]);
            }
            printf("\n");		
            if(outAttr && outAttr->value.aclfield.data.objlist.list)
            {
                outAttr->value.aclfield.data.objlist.count = value.aclfield.data.objlist.count;
                memcpy(outAttr->value.aclfield.data.objlist.list, 
                        value.aclfield.data.objlist.list,
                        sizeof(sai_object_id_t)*(value.aclfield.data.objlist.count)); 
            }
        }
        else if(strcmp(attr_type , "sai_u8_list_t") ==0)
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("mask =");
            for (j=0;j< value.aclfield.mask.u8list.count ; j++)
            {
                printf("%d  ", value.aclfield.mask.u8list.list[j]);
            }
            printf("\n");

            printf("data =");
            for (j=0;j< value.aclfield.data.u8list.count ; j++)
            {
                printf("%d  ", value.aclfield.data.u8list.list[j]);
            }
            printf("\n");
            if(outAttr && outAttr->value.aclfield.data.u8list.list)
            {
                outAttr->value.aclfield.data.u8list.count = value.aclfield.data.u8list.count;
                memcpy(outAttr->value.aclfield.data.u8list.list, 
                        value.aclfield.data.u8list.list,
                        sizeof(uint8_t)*(value.aclfield.data.u8list.count)); 
            }
        }
        else
        {
            printf("enable = %d\n",(int) value.aclfield.enable);
            printf("data = %d\n", (int) value.aclfield.data.s32);
        }
    }
    else if( strcmp(maintype, "sai_acl_action_data_t") == 0 )
    {	
        if(outAttr)
        {
            memcpy(&(outAttr->value.aclaction), &(value.aclaction), 
                    (sizeof(sai_acl_action_data_t)));
        }
        if(strcmp(attr_type, "sai_uint8_t") ==0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.u8); 
        }

        else if(strcmp(attr_type, "sai_int8_t") == 0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.s8);
        }

        else if(strcmp(attr_type, "sai_uint16_t") ==0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.u16); 
        }

        else if(strcmp(attr_type, "sai_int16_t") == 0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.s16);
        }
        else if(strcmp(attr_type, "sai_uint32_t") ==0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.u32); 
        }

        else if(strcmp(attr_type, "sai_int32_t") == 0)
        {   printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n",(int) value.aclaction.parameter.s32);
        }
        else if(strcmp(attr_type , "sai_mac_t")==0)
        {
            printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter =");
            for (j=0;j<5;j++)
                printf("%d:", value.aclaction.parameter.mac[j]);
            printf("%d\n",value.aclaction.parameter.mac[j]);

        }
        else if(strcmp(attr_type , "sai_ip4_t")==0)
        {
            uint32_t len = sizeof(sai_ip4_t);
            printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter =");
            addr = value.aclaction.parameter.ip4;
            for(j = 0; j < len-1; j++)
            {
                printf("%d.", ((uint8_t*)&addr)[j]);
            }
            printf("%d\n", ((uint8_t*)&addr)[j]);
        }
        else if(strcmp(attr_type , "sai_ip6_t")==0)
        {
            printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter =");
            for (j=0;j<15;j++)
                printf("%d:", value.aclaction.parameter.ip6[j]);
            printf("%d",value.aclaction.parameter.ip6[j]);
        }
        else if(strcmp(attr_type , "sai_object_id_t") == 0)
        {
            printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = ");
            printf("%" PRIu64 "\n", (uint64_t) value.aclaction.parameter.oid);
        }
        else if(strcmp(attr_type , "sai_object_list_t") ==0)
        {
            uint64_t temp;
            printf("enable = %d\n",(int) value.aclaction.enable);
            for (j=0;j< value.aclaction.parameter.objlist.count ; j++)
            {
                temp = value.aclaction.parameter.objlist.list[j];
                printf("%" PRIu64 " ", (const uint64_t) temp);
            }
            printf("\n");		
            if(outAttr && outAttr->value.aclaction.parameter.objlist.list)
            {
                outAttr->value.aclaction.parameter.objlist.count = value.aclaction.parameter.objlist.count;
                memcpy(outAttr->value.aclfield.data.u8list.list, 
                        value.aclaction.parameter.objlist.list,
                        sizeof(sai_object_id_t)*(value.aclaction.parameter.objlist.count)); 
            }
        }
        else
        {
            printf("enable = %d\n",(int) value.aclaction.enable);
            printf("parameter = %d\n", (int) value.aclaction.parameter.s32);
        }

    }   
    else
    {
        print_attribute(attr_type, value, outAttr);
    }
}	

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sai.h>

#ifdef __cplusplus
extern "C" {
#endif
void print_attribute(char *attr_type , sai_attribute_value_t value, sai_attribute_t *outAttr);


void acl_print_attribute(char* maintype, char* type, sai_attribute_value_t value, sai_attribute_t *outAttr);
#ifdef __cplusplus
}
#endif

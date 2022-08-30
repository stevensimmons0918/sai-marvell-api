#ifndef _SCHED_REGISTER_INTERFACE_H_
#define _SCHED_REGISTER_INTERFACE_H_

/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief function declarations  and macros for   dealing with  register payloads and  register definition macro tables.
 *
* @file tm_register_processing.h
*
* $Revision: 2.0 $
 */



int schedSetField
(
    void    *address,
    int     shift,
    int     width,
    int     value,
    char    *fieldName
);


unsigned int schedGetField(void * address, int shift, int width);





/*
 workaround for MSVS variadic macros bug - all variadic parameters are passed inside macro as 1 parameter (first in list) other are empty
 so I use __VA_ARGS__ inside macro  and following macros in order to extract parameters from VA_ARGS_
 The following dummy macro is used because this action needs additional macro nesting level (???)
*/

#define DUMMY_MACRO(x) x

#define PARAM_1_OF_4(a,b,c,d) a
#define PARAM_2_OF_4(a,b,c,d) b
#define PARAM_3_OF_4(a,b,c,d) c
#define PARAM_4_OF_4(a,b,c,d) d

#define PARAM_1_OF_3(a,b,c) a
#define PARAM_2_OF_3(a,b,c) b
#define PARAM_3_OF_3(a,b,c) c

#define PARAM_1_OF_2(a,b) a
#define PARAM_2_OF_2(a,b) b

/* intermediate macros */

#define	 TM_REG_ROW_SET(field_name,shift,width,default_value, ...) \
    else if (cpssOsStrCmp(DUMMY_MACRO(PARAM_1_OF_3(__VA_ARGS__)),#field_name)==0) \
    {\
        int setRc;\
        if((setRc = schedSetField(TM_REGISTER_VAR_PTR(PARAM_2_OF_3(__VA_ARGS__)), shift, width, DUMMY_MACRO(PARAM_3_OF_3(__VA_ARGS__)),#field_name))!=0)\
            {\
                return setRc;\
            }\
    }

#define  TM_REG_ROW_RESET(field_name,shift,width,default_value,...) schedSetField(TM_REGISTER_VAR_ADDR(__VA_ARGS__), shift, width, default_value,#field_name);

#define  TM_REG_ROW_GET(field_name,shift,width,default_value, ...) \
    else if (cpssOsStrCmp(DUMMY_MACRO(PARAM_1_OF_4(__VA_ARGS__)),#field_name)==0)\
    {\
        DUMMY_MACRO(PARAM_3_OF_4(__VA_ARGS__)) = DUMMY_MACRO(PARAM_4_OF_4(__VA_ARGS__)) schedGetField(TM_REGISTER_VAR_PTR(PARAM_2_OF_4(__VA_ARGS__)), shift, width );\
    }

#define  TM_REG_ROW_DUMP(field_name,shift,width,default_value,...)  \
        fprintf( DUMMY_MACRO(PARAM_2_OF_2(__VA_ARGS__)),"    %s (shift=%d,width=%d) value=%u\n",#field_name,shift,width,schedGetField(TM_REGISTER_VAR_PTR(PARAM_1_OF_2(__VA_ARGS__)),shift,width));


/*
 the register variable of some register type must be unique in the scope of it's definition
 It's name is defined by register name in the following macro.  Two following macros should be used everywhere instead of direct usage of refister variable name and address
*/
#define	TM_REGISTER_VAR_NAME(register) register##_var
#define	TM_REGISTER_VAR_ADDR(register) TM_REGISTER_VAR_PTR(TM_REGISTER_VAR_NAME(register))

/* if use register description using shifts: */
#if defined(SHIFT_TABLE)

	/* the register variable is defined as 8 bytes memory block */
	#define	TM_REGISTER_VAR(register_name) char TM_REGISTER_VAR_NAME(register_name)[8]= {0,0,0,0,0,0,0,0};

	#define	TM_REGISTER_VAR_PTR(register_var)  (void*)register_var


    #define TXQ_SCHED_DEVDEPEND_REGISTER_VAR(register_name) TM_REGISTER_VAR(TM_Sched_##register_name)\
                                              TM_REGISTER_VAR(PDQ_SIP_6_30_##register_name)



    #define	 TXQ_SCHED_REGISTER_SET(register, param_name, param_value) \
	{\
		if (0) ;\
		register(SET, #param_name, TM_REGISTER_VAR_NAME(register), param_value)\
		else	{ 	cpssOsFprintf(CPSS_OS_FILE_STDERR,"%s line:%d unknown parameter %s used in assignment.\n",__FILE__,__LINE__,#param_name); cpssOsFatalError(CPSS_OS_FATAL_RESET,"aborting\n");}\
	}


    #define  TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(register, param_name, param_value) \
    {\
        PDQ_ENV_MAGIC_CHECK(ctl)\
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
        {\
            TXQ_SCHED_REGISTER_SET(TM_Sched_##register, param_name, param_value)\
        }\
        else\
        {\
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_6_30_##register, param_name, param_value)\
        }\
    }\



	#define	 TXQ_SCHED_REGISTER_GET(register, param_name, param_value, casting) \
	{\
		if (0) ;\
		register(GET, #param_name, TM_REGISTER_VAR_NAME(register), param_value , casting)\
		else	{ 	cpssOsFprintf(CPSS_OS_FILE_STDERR,"%s line:%d unknown parameter %s used in read request.\n",__FILE__,__LINE__,#param_name); cpssOsFatalError(CPSS_OS_FATAL_RESET,"aborting\n"); } \
	}


#define  TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(register, param_name, param_value,casting) \
    {\
        PDQ_ENV_MAGIC_CHECK(ctl)\
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
        {\
            TXQ_SCHED_REGISTER_GET(TM_Sched_##register, param_name, param_value,casting)\
        }\
        else\
        {\
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_6_30_##register, param_name, param_value,casting)\
        }\
    }\


	#define	TXQ_SCHED_REGISTER_RESET(register) \
	{\
		cpssOsMemSet(TM_REGISTER_VAR_ADDR(register),0,sizeof(char)*8);\
		register(RESET,register)\
	}

    #define TXQ_SCHED_REGISTER_DEVDEPEND_RESET(register) \
    {\
        PDQ_ENV_MAGIC_CHECK(ctl)\
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
        {\
            TXQ_SCHED_REGISTER_RESET(TM_Sched_##register)\
        }\
        else\
        {\
            TXQ_SCHED_REGISTER_RESET(PDQ_SIP_6_30_##register)\
        }\
        rc =0;\
    }\


	#define	 TM_REGISTER_DUMP(register, file_handle) \
	{\
		fprintf(file_handle," register %s:\n",#register);\
		register(DUMP,TM_REGISTER_VAR_NAME(register),file_handle)\
	}

#else
   /* error here - data declaration method must be selected */
#endif

#endif /* _TM_REGISTER_INTERFACE_H_ */

// xpSaiProfileMng.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

/*******************************************************************************
* @file xpSaiProfileMng.h
*
* @brief Private API implementation for CPSS ACL feature which can be used in
*        XPS layer.
*
* @version   01
*******************************************************************************/

#include "xpSaiWred.h"

#define PROFILE_MNG_PER_PORT_CFG 0xFF
#define PROFILE_MNG_MAX_TC 8

//Func: xpSaiQueueKeyComp
#define DYN_TH_2_ALFA(_th,_alfa,_error) \
    do\
    {\
        if(_th>=0)\
        {\
            switch(_th)\
            {\
                case 0:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;\
                    break;\
                case 1:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;\
                    break;\
                case 2:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;\
                    break;\
                 case 3:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;\
                    break;\
                 case 4:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E;\
                    break;\
                case 5:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;\
                    break;\
                default:\
                    _error= GT_TRUE;\
            }\
        }\
        else\
        {\
            _th = _th*(-1);\
            switch(_th)\
            {\
                case 1:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;\
                    break;\
                case 2:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;\
                    break;\
                 case 3:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;\
                    break;\
                 case 4:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E;\
                    break;\
                case 5:\
                    _alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E;\
                    break;\
                default:\
                    _error= GT_TRUE;\
            }\
        }\
    }\
    while(0);\


typedef struct
{
    CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC
    queueWredProfileParams[PROFILE_MNG_MAX_TC];
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC
    queueTdProfileParams[PROFILE_MNG_MAX_TC];
    CPSS_PORT_QUEUE_ECN_PARAMS_STC
    queueEcnProfileParams[PROFILE_MNG_MAX_TC];
    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC             portWredProfileParams;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT            portAlpha;
} xpSaiProfileMngPortAttr_t;



XP_STATUS xpSaiProfileMngConfigureGuaranteedLimitAndAlfa
(xpsDevice_t devId,
 xpsPort_t devPort,
 uint32_t queueNum,
 uint32_t *lengthMaxThresholdPtr,
 CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *alphaPtr
);

XP_STATUS xpSaiProfileMngConfigureWredAndEcn
(IN xpsDevice_t devId,
 IN xpsPort_t devPort,
 IN uint32_t queueNum,
 IN xpSaiWred_t *pWredInfo
);






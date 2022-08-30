// xpSaiStub.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*
Generic stub api the list of function pointers which are not registered with any SAI API.
*/

sai_status_t xpSaiStubGenericApi(sai_object_id_t object_id, ...);

#ifdef __cplusplus
}
#endif


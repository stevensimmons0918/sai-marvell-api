// xpsExceptions.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef XP_EXCEPTIONS_H
#define XP_EXCEPTIONS_H

#define xpTry        try
#define xpCatch(X)   catch(X)
#define xpThrow(X)   throw(X)
#define xpThrowErr(X)   ;

typedef struct _xpExpVar
{
    int tid;
    int expVar;
} xpExpVar_t;

#define MAX_XP_EXP_TASKS    (10)

/* try catch exception functions() */
extern xpExpVar_t xpVarArr[];
extern int xpExpTry();
extern int xpExpCatch();
extern void setXpExpVar(int var);
extern int getXpExpVar();
extern void resetXpExpVar();
extern void resetAllXpExpVar();
extern void xpExpThrow(int n);

#endif /* XP_EXCEPTIONS_H */

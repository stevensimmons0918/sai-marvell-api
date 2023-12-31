/* log_impl.h
 *
 * Log Module, implementation for User Mode
 */

/*****************************************************************************
* Copyright (c) 2008-2016 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

#ifndef INCLUDE_GUARD_LOG_IMPL_H
#define INCLUDE_GUARD_LOG_IMPL_H

#define Log_Message           cpssOsPrintf
#define Log_FormattedMessage  cpssOsPrintf

/* backwards compatible implementation */
#define Log_FormattedMessageINFO  CPSS_LOG_INFORMATION_MAC
#define Log_FormattedMessageWARN  CPSS_LOG_INFORMATION_MAC
#define Log_FormattedMessageCRIT  CPSS_LOG_ERROR_MAC

#endif /* Include Guard */

/* end of file log_impl.h */

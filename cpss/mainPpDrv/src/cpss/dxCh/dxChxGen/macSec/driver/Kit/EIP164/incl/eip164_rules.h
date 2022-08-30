/* eip164_rules.h
 *
 * EIP-164 Driver Library Packet Classification Rules interface
 *
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.5                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2020-Feb-27                                              */
/*                                                                            */
/* Copyright (c) 2008-2020 INSIDE Secure B.V. All Rights Reserved             */
/*                                                                            */
/* This confidential and proprietary software may be used only as authorized  */
/* by a licensing agreement from INSIDE Secure.                               */
/*                                                                            */
/* The entire notice above must be reproduced on all authorized copies that   */
/* may only be made to the extent permitted by a licensing agreement from     */
/* INSIDE Secure.                                                             */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

#ifndef EIP164_RULES_H_
#define EIP164_RULES_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

/* EIP-164 Driver Library Types API */
#include <Kit/EIP164/incl/eip164_types.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * EIP164_Rules_SecTag_Update
 *
 * This function updates the rules for the classification of packets using the
 * SecTAG field for the EIP-164 device identified by IOArea_p parameter.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *     The channel number/index to set the SecTAG parsing rules for.
 *
 * SecTag_Rules_p (input)
 *     Pointer to a memory location where the SecTAG parsing and compare rules
 *     for each packet are specified.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other Rules API
 * function for the same or different Device.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Rules_SecTag_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const EIP164_Channel_Rule_SecTAG_t * const SecTag_Rules_p);


/*----------------------------------------------------------------------------
 * EIP164_Rules_SecTag_Read
 *
 * This function reads the rules for the classification of packets using the
 * SecTAG field for the EIP-164 device identified by IOArea_p parameter.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *     The channel number/index to read the SecTAG parsing rules for.
 *
 * SecTag_Rules_p (input)
 *     Pointer to a memory location where the SecTAG parsing and compare rules
 *     for each packet are stored.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other Rules API
 * function for the same or different Device.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Rules_SecTag_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        EIP164_Channel_Rule_SecTAG_t * const SecTag_Rules_p);


#endif /* EIP164_RULES_H_ */


/* end of file eip164_rules.h */

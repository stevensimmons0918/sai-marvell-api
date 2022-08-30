// xpsUtil.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsUtil.h
 * \brief
 *
 */

#ifndef _xpsUtil_h_
#define _xpsUtil_h_

#include "openXpsTypes.h"
#include "xpTypes.h"
#include "xpsCopp.h"
#include "xpsEnums.h"
#include "xpsCommon.h"
#include <gtOs/gtGenTypes.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CPU_CODE_COUNT 8

typedef enum xpLogLevel
{
    XP_LOG_DEBUG,
    XP_LOG_TRACE,
    XP_LOG_DEFAULT,
    XP_LOG_WARNING,
    XP_LOG_ERROR,
    XP_LOG_CRITICAL,
    XP_LOG_MAX_LEVEL
} xpLogLevel;



/*
 * DLL utility
 */

/**
 * \typedef xpsDllLink
 * \brief This struct is used to create a doubly linked list node
 *
 * Various states will use this struct for linked lists
 */

typedef struct xpsDllLink
{
    struct xpsDllLink  *prev;
    struct xpsDllLink  *next;
    void               *data;
} xpsDllLink;

/**
 * \typedef xpsDllHead
 * \brief This struct is used to create a doubly linked list
 *        head
 *
 * Various states will use this struct for linked lists
 */

typedef struct xpsDllHead
{
    xpsDllLink         *first;
    xpsDllLink         *last;
} xpsDllHead;

/**
 * \brief Adds a node at the end of the doubly linked list
 *
 *
 * \param [in] *head
 * \param [in] *node
 */

void xpsDllLinkAtTheEnd(xpsDllHead *head, xpsDllLink *node);

/**
 * \brief Unlinks a node from a doubly linked list
 *
 *
 * \param [in] *head
 * \param [in] *node
 */
void xpsDllUnlink(xpsDllHead *head, xpsDllLink *node);


/*
 * Device map utility
 */

/**
 * \typedef xpsDeviceMap
 * \brief This struct is used to maintain a bit mask of devices
 *        used in the system
 *
 * Various states will use this struct for maintenance
 */
typedef struct xpsDeviceMap
{
    uint8_t  bits[XP_MAX_DEVICES/SIZEOF_BYTE];
} xpsDeviceMap;


/**
 * \brief Sets a bit into the device map indicating a device's
 *        presence
 *
 *
 * \param [in] * deviceMap
 * \param [in] devId
 */
void xpsDeviceMapSetDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId);

/**
 * \brief Clears a bit from the device map indicating that a
 *        device is not present
 *
 *
 * \param [in] * deviceMap
 * \param [in] devId
 */
void xpsDeviceMapClrDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId);

/**
 * \brief Determines if a device is present in the device map
 *
 *
 * \param [in] * deviceMap
 * \param [in] devId
 *
 * \return uint32_t
 */
uint32_t xpsDeviceMapTestDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId);

/**
 * \brief Determines if the device map is empty, no devices are
 *        present
 *
 *
 * \param [in] * deviceMap
 *
 * \return uint32_t
 */
uint32_t xpsDeviceMapIsEmpty(xpsDeviceMap *deviceMap);


/**
 * \brief Determines if the given IPv4 Address is a multicast IP
 *
 *
 * \param [in] addr
 *
 * \return uint32_t
 */
uint32_t xpsIsIPv4MulticastAddress(ipv4Addr_t addr);

/**
 * \brief Check if the number of power of Two
 *
 *
 * \param [in] addr
 *
 * \return uint32_t
 */
uint32_t xpsIsPowerOfTwo(uint32_t number);

/**
 * \brief Check if the context/structure which contains the
 *          dynamic array needs more memory.
 *
 *
 * \param [in] numOfArrayElements
 * \param [in] defaultArraySize
 *
 * \return uint32_t
 */
uint32_t xpsDAIsCtxGrowthNeeded(uint16_t numOfArrayElements,
                                uint16_t defaultArraySize);
/**
 * \brief Check if the context/structure which contains the
 *          dynamic array needs to be shrinked.
 *
 *
 * \param [in] numOfArrayElements
 * \param [in] defaultArraySize
 *
 * \return uint32_t
 */
uint32_t xpsDAIsCtxShrinkNeeded(uint16_t numOfArrayElements,
                                uint16_t defaultArraySize);
/**
 * \brief Get the memory needed by the context/structure containing the
 *          dynamic array to be expanded.
 *
 *
 * \param [in] sizeOfContext
 * \param [in] sizeOfElement
 * \param [in] numOfArrayElements
 * \param [in] defaultArraySize
 *
 * \return uint32_t
 */
uint32_t xpsDAGetCtxSizeWhileGrow(uint16_t sizeOfContext,
                                  uint16_t sizeOfElement, uint16_t numOfArrayElements, uint16_t defaultArraySize);
/**
 * \brief Get the memory needed by the context/structure containing the
 *          dynamic array to be shrinked.
 *
 *
 * \param [in] sizeOfContext
 * \param [in] sizeOfElement
 * \param [in] numOfArrayElements
 * \param [in] defaultArraySize
 *
 * \return uint32_t
*/
uint32_t xpsDAGetCtxSizeWhileShrink(uint16_t sizeOfContext,
                                    uint16_t sizeOfElement, uint16_t numOfArrayElements, uint16_t defaultArraySize);

/**
 * \brief Print the detailed boot time.
 *
 * \param [in] devId device Id. Valid values are 0-63
 *
*/
void xpsPrintBootTimeAnalysis(xpsDevice_t devId);

/**
 * \brief Check if the vlan Id valid
 *
 * \param [in] vlanId Vlan id value
 */
uint8_t xpsIsVlanIdValid(xpsVlan_t vlanId);

/**
 * \brief Handle the port vlan table rehash.
 *
 * \param [in] devId
 * \param [in] indexList List of rehashed indices
 */
XP_STATUS xpsHandlePortVlanTableRehash(xpsDevice_t devId,
                                       xpsHashIndexList_t *indexList);

/**
 * \brief Handle the tunnel Ivif table rehash.
 *
 * \param [in] devId
 * \param [in] indexList List of rehashed indices
 */
XP_STATUS xpsHandleTunnelIvifTableRehash(xpsDevice_t devId,
                                         xpsHashIndexList_t *indexList);

/**
 * \brief Converter from xps Reason Code to Cpss CPU Code.
 *
 * \param [in] pXpsTrap xps Reason Code
 * \param [in] *xpsTrapType pointer to store cpss CPU Code
 * \param [in] count        number of cpu code stored
 */
XP_STATUS xpsUtilConvertReasonCodeToCpssCpuCode(xpsCoppCtrlPkt_e pXpsTrap,
                                                CPSS_NET_RX_CPU_CODE_ENT reasonCode[], uint8_t* count);

int xpsGetExitStatus();
void xpsSetExitStatus();

void writeLog(char* fileName, int lineNum, xpLogLevel msgLogLevel,
              const char* msg, ...);



/**
 * \brief Gets the log level
 *
 *
 * \param [in] void
 *
 * \return xpLogLevel
 */

xpLogLevel xpsGetLogLevel(void);


/**
 * \brief Gets the number of log levels
 *
 *
 * \param [in] void
 *
 * \return uint8_t
 */

uint8_t xpsGetNumLogLevels(void);


/**
 * \brief Set the log level
 *
 *
 * \param [in] xpLoglevel
 *
 * \return XP_STATUS
 */

GT_STATUS xpsSetLogLevel(xpLogLevel l);


/**
 * \brief Gets the log level name
 *
 *
 * \param [in] xpLogLevel
 *
 * \return char*
 */

char* xpsGetLogLevelName(xpLogLevel l);



/**
 * \brief Checks falcon device or not
 *
 *
 * \param [in] devId
 *
 * \return uint8_t
 */
uint8_t xpsIsDeviceFalcon(xpsDevice_t devId);

XP_STATUS xpsDynamicArrayGrow(void **newContext, void *oldContext,
                              uint32_t sizeOfContext,
                              uint16_t sizeOfElement,
                              uint32_t numOfElements,
                              uint32_t defaultSize);
XP_STATUS xpsDynamicArrayShrink(void **newContext,
                                void *oldContext,
                                uint32_t sizeOfContext,
                                uint16_t sizeOfElement,
                                uint32_t numOfElements,
                                uint32_t defaultSize);
XP_STATUS xpsUtilCpssShellEnable(bool enable);
#ifdef __cplusplus
}
#endif

#endif //_xpsUtil_h_


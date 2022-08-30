// xpsVsi.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsVsi.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Virtual Service Instance Manager
 * \Copyright (c) Marvell [2000-2020]. All rights reservered. Confidential.. ALL RIGHTS RESERVED.
 */
#ifndef _xpsVsi_h_
#define _xpsVsi_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This API initializes xps VSI manager
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiInit();

/**
 * \brief This API initializes xps VSI manager with specific scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiInitScope(xpsScope_t scopeId);

/**
 * \brief This API de-initializes xps VSI manager
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiDeinit();

/**
 * \brief This API de-initializes xps VSI manager with specific scope
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiDeinitScope(xpsScope_t scopeId);

/**
 * \brief This API sets up the resources for VSI manager for the given design
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This API releases the resources for VSI manager for the given design
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiRemoveDevice(xpsDevice_t devId);

/**
 * \brief This API creates a VSI with an access mode
 *
 * \param [in] accessMode
 * \param [out] vsiId       VSI id of the VSI created
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiCreate(xpsVsiAccessMode_e accessMode, xpsVsi_t *vsiId);

/**
 * \brief This API creates a VSI with an access mode in a specific scope
 *
 * \param [in] scopeId
 * \param [in] accessMode
 * \param [out] vsiId       VSI id of the VSI created
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiCreateScope(xpsScope_t scopeId, xpsVsiAccessMode_e accessMode,
                            xpsVsi_t *vsiId);

/**
 * \brief This API destroys a VSI
 *
 * \param [in] vsiId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiDestroy(xpsVsi_t vsiId);

/**
 * \brief This API destroys a VSI in a specific scope
 *
 * \param [in] scopeId
 * \param [in] vsiId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiDestroyScope(xpsScope_t scopeId, xpsVsi_t vsiId);

/**
 * \brief This API adds a VSI to a specific device.
 *
 * The VSI should have been created with xpsVsiCreate
 *
 * \param [in] devId
 * \param [in] vsiId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiAdd(xpsDevice_t devId, xpsVsi_t vsiId);

/**
 * \brief This API removes a VSI from a specific device.
 *
 * The VSI should have been created with xpsVsiCreate
 *
 * \param [in] devId
 * \param [in] vsiId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiRemove(xpsDevice_t devId, xpsVsi_t vsiId);

/**
 * \brief This API adds an Attachment Circuit to the VSI in a devices.
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [in] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiAddAc(xpsDevice_t devId, xpsVsi_t vsiId,
                      xpsInterfaceId_t acIntfId);

/**
 * \brief This API adds an Attachment Circuit from a VSI in a devices.
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [in] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiRemoveAc(xpsDevice_t devId, xpsVsi_t vsiId,
                         xpsInterfaceId_t acIntfId);

/**
 * \brief This API adds an tunnel interface to the VSI in a devices.
 *
 * Only VXLAN tunnel is supported
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [in] acIntfId
 * \param [in] vniId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiAddTunnel(xpsDevice_t devId, xpsVsi_t vsiId,
                          xpsInterfaceId_t tnlIntfId, uint32_t vniId);

/**
 * \brief This API removes an tunnel interface from aVSI in a devices.
 *
 * Only VXLAN tunnel is supported
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [in] acIntfId
 * \param [in] vniId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiRemoveTunnel(xpsDevice_t devId, xpsVsi_t vsiId,
                             xpsInterfaceId_t tnlIntfId, uint32_t vniId);

/**
 * \brief This API sets unknown SA command for a VSI
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [in] saMissCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiSetUnknownSaCmd(xpsDevice_t devId, xpsVsi_t vsiId,
                                xpsPktCmd_e saMissCmd);

/**
 * \brief This API gets unknown SA command for a VSI
 *
 * \param [in] devId
 * \param [in] vsiId
 * \param [out] saMissCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVsiGetUnknownSaCmd(xpsDevice_t devId, xpsVsi_t vsiId,
                                xpsPktCmd_e *saMissCmd);

#ifdef __cplusplus
}
#endif

#endif  //_xpsVsi_h_

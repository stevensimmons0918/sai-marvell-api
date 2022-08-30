// xpsAc.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsAc.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Attachment Circuit Manager
 * \Copyright (c) Marvell [2000-2020]. All rights reservered. Confidential.. ALL RIGHTS RESERVED.
 */
#ifndef _xpsAc_h_
#define _xpsAc_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct xpsAcInfo_t
 * \brief This structure defines the Attachment circuit properties
 */
typedef struct xpsAcInfo_t
{
    xpsAcMatchType_e    matchType;      ///< Matchtype of the AC
    xpsInterfaceId_t    interfaceId;    ///< AC's base Physical or lag interface
    xpVlan_t            outerVid;       ///< Vlan id for VID match AC types only
} xpsAcInfo_t;

/**
 * \struct xpsAcConfig_t
 * \brief This structure defines the Attachment circuit configuration parameters
 */
typedef struct xpsAcConfig_t
{
    uint8_t baclEn;
    uint8_t raclEn;
    uint32_t baclId;
    uint32_t raclId;
} xpsAcConfig_t;

/**
 * \brief This API initializes the AC manager
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcInit();

/**
 * \brief This API initializes the AC manager in a specifc sccope
 *
 * \param [in] scope
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcInitScope(xpsScope_t scope);

/**
 * \brief This API De-initializes the AC manager
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcDeinit();

/**
 * \brief This API De-initializes the AC manager in a specific scope
 *
 * \param [in] scope
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcDeinitScope(xpsScope_t scope);

/**
 * \brief This API sets up the resources for AC manager for a specific device
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This API releases up the resources for AC manager for a specific device
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcRemoveDevice(xpsDevice_t devId);

/**
 * \brief This API creates an attachment circuit with the given info
 *
 * \param [in] acInfo
 * \param [out] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcCreate(xpsAcInfo_t *acInfo, xpsInterfaceId_t *acIntfId);

/**
 * \brief This API creates an attachment circuit with the given info in a specific scope
 *
 * \param [in] scope
 * \param [in] acInfo
 * \param [out] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcCreateScope(xpsScope_t scope, xpsAcInfo_t *acInfo,
                           xpsInterfaceId_t *acIntfId);

/**
 * \brief This API destroys an attachment circuit
 *
 * \param [in] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcDestroy(xpsInterfaceId_t acIntfId);

/**
 * \brief This API destroys an attachment circuit in a specific scope
 *
 * \param [in] scope
 * \param [in] acIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcDestroyScope(xpsScope_t scope, xpsInterfaceId_t acIntfId);

/**
 * \brief This API sets configurations for an attachment circuit
 *
 * \param [in] devId
 * \param [in] acIntfId
 * \param [in] config
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcSetConfig(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                         xpsAcConfig_t *config);

/**
 * \brief This API gets configurations for an attachment circuit
 *
 * \param [in] devId
 * \param [in] acIntfId
 * \param [out] config
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcGetConfig(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                         xpsAcConfig_t *config);

#ifdef __cplusplus
}
#endif

#endif  //_xpsAc_h_

/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

/*!
 * Header file containing the public API for the System Controller (SC)
 * Miscellaneous (MISC) function.
 *
 * @addtogroup MISC_SVC (SVC) Miscellaneous Service
 *
 * Module for the Miscellaneous (MISC) service.
 *
 * @{
 */

#ifndef SC_MISC_API_H
#define SC_MISC_API_H

/* Includes */

#include <asm/mach-imx/sci/types.h>
#include <asm/mach-imx/sci/svc/rm/api.h>

/* Defines */

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_MISC_DMA_GRP_W       5U      /*!< Width of sc_misc_dma_group_t */
/*@}*/

/*! Max DMA channel priority group */
#define SC_MISC_DMA_GRP_MAX     31U

/*!
 * @name Defines for sc_misc_boot_status_t
 */
/*@{*/
#define SC_MISC_BOOT_STATUS_SUCCESS     0U   /*!< Success */
#define SC_MISC_BOOT_STATUS_SECURITY    1U   /*!< Security violation */
/*@}*/

/*!
 * @name Defines for sc_misc_temp_t
 */
/*@{*/
#define SC_MISC_TEMP                    0U   /*!< Temp sensor */
#define SC_MISC_TEMP_HIGH               1U   /*!< Temp high alarm */
#define SC_MISC_TEMP_LOW                2U   /*!< Temp low alarm */
/*@}*/

/*!
 * @name Defines for sc_misc_seco_auth_cmd_t
 */
/*@{*/
#define SC_MISC_AUTH_CONTAINER          0U   /*!< Authenticate container */
#define SC_MISC_VERIFY_IMAGE            1U   /*!< Verify image */
#define SC_MISC_REL_CONTAINER           2U   /*!< Release container */
#define SC_MISC_SECO_AUTH_SECO_FW       3U   /*!< SECO Firmware */
#define SC_MISC_SECO_AUTH_HDMI_TX_FW    4U   /*!< HDMI TX Firmware */
#define SC_MISC_SECO_AUTH_HDMI_RX_FW    5U   /*!< HDMI RX Firmware */
/*@}*/

/*!
 * @name Defines for sc_misc_bt_t
 */
/*@{*/
#define SC_MISC_BT_PRIMARY              0U
#define SC_MISC_BT_SECONDARY            1U
#define SC_MISC_BT_RECOVERY             2U
#define SC_MISC_BT_MANUFACTURE          3U
#define SC_MISC_BT_SERIAL               4U
/*@}*/

/* Types */

/*!
 * This type is used to store a DMA channel priority group.
 */
typedef uint8_t sc_misc_dma_group_t;

/*!
 * This type is used report boot status.
 */
typedef uint8_t sc_misc_boot_status_t;

/*!
 * This type is used to issue SECO authenticate commands.
 */
typedef uint8_t sc_misc_seco_auth_cmd_t;

/*!
 * This type is used report boot status.
 */
typedef uint8_t sc_misc_temp_t;

/*!
 * This type is used report the boot type.
 */
typedef uint8_t sc_misc_bt_t;

/* Functions */

/*!
 * @name Control Functions
 * @{
 */

/*!
 * This function sets a miscellaneous control value.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource the control is associated with
 * @param[in]     ctrl        control to change
 * @param[in]     val         value to apply to the control
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner or parent
 *   of the owner
 *
 * Refer to the [Control List](@ref CONTROLS) for valid control values.
 */
sc_err_t sc_misc_set_control(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_ctrl_t ctrl, uint32_t val);

/*!
 * This function gets a miscellaneous control value.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource the control is associated with
 * @param[in]     ctrl        control to get
 * @param[out]    val         pointer to return the control value
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner or parent
 *   of the owner
 *
 * Refer to the [Control List](@ref CONTROLS) for valid control values.
 */
sc_err_t sc_misc_get_control(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_ctrl_t ctrl, uint32_t *val);

/* @} */

/*!
 * @name DMA Functions
 * @{
 */

/*!
 * This function configures the max DMA channel priority group for a
 * partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to assign \a max
 * @param[in]     max         max priority group (0-31)
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the parent
 *   of the affected partition
 *
 * Valid \a max range is 0-31 with 0 being the lowest and 31 the highest.
 * Default is the max priority group for the parent partition of \a pt.
 */
sc_err_t sc_misc_set_max_dma_group(sc_ipc_t ipc, sc_rm_pt_t pt,
    sc_misc_dma_group_t max);

/*!
 * This function configures the priority group for a DMA channel.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    DMA channel resource
 * @param[in]     group       priority group (0-31)
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the owner or parent
 *   of the owner of the DMA channel
 *
 * Valid \a group range is 0-31 with 0 being the lowest and 31 the highest.
 * The max value of \a group is limited by the partition max set using
 * sc_misc_set_max_dma_group().
 */
sc_err_t sc_misc_set_dma_group(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_misc_dma_group_t group);

/* @} */

/*!
 * @name Security Functions
 * @{
 */

/*!
 * This function loads a SECO image.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr_src    address of image source
 * @param[in]     addr_dst    address of image destination
 * @param[in]     len         lenth of image to load
 * @param[in]     fw          SC_TRUE = firmware load
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if word fuse index param out of range or invalid
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This is used to load images via the SECO. Examples include SECO
 * Firmware and IVT/CSF data used for authentication. These are usually
 * loaded into SECO TCM. \a addr_src is in secure memory.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_image_load(sc_ipc_t ipc, sc_faddr_t addr_src,
    sc_faddr_t addr_dst, uint32_t len, sc_bool_t fw);

/*!
 * This function is used to authenticate a SECO image or command.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     cmd         authenticate command
 * @param[in]     addr        address of/or metadata
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if word fuse index param out of range or invalid
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This is used to authenticate a SECO image or issue a security
 * command. \a addr often points to an container. It is also
 * just data (or even unused) for some commands.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_authenticate(sc_ipc_t ipc,
    sc_misc_seco_auth_cmd_t cmd, sc_faddr_t addr);

/*!
 * This function is used to load a SECO key.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     id          key identifier
 * @param[in]     addr        key address
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if word fuse index param out of range or invalid
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This function is used to install private cryptographic keys encapsulated
 * in a blob previously generated by SECO. The controller can be either the
 * IEE or the VPU. The blob header carries the controller type and the key
 * size, as provided by the user when generating the key blob.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_load_key(sc_ipc_t ipc, uint32_t id,
    sc_faddr_t addr);

/*!
 * This function securely writes a group of fuse words.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address of message block
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * Note \a addr must be a pointer to a signed message block.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_fuse_write(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function securely enables debug.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address of message block
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * Note \a addr must be a pointer to a signed message block.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_enable_debug(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function updates the lifecycle of the device.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     change      desired lifecycle transistion
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This message is used for going from Open to NXP Closed to OEM Closed.
 * Note \a change is NOT the new desired lifecycle. It is a lifecycle
 * transition as documented in the Security Reference Manual (SRM).
 *
 * If any SECO request fails or only succeeds because the part is in an
 * "OEM open" lifecycle, then a request to transition from "NXP closed"
 * to "OEM closed" will also fail. For example, booting a signed container
 * when the OEM SRK is not fused will succeed, but as it is an abnormal
 * situation, a subsequent request to transition the lifecycle will return
 * an error.
 */
sc_err_t sc_misc_seco_forward_lifecycle(sc_ipc_t ipc, uint32_t change);

/*!
 * This function updates the lifecycle to one of the return lifecycles.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address of message block
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * Note \a addr must be a pointer to a signed message block.
 *
 * To switch back to NXP states (Full Field Return), message must be signed
 * by NXP SRK. For OEM States (Partial Field Return), must be signed by OEM
 * SRK.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_return_lifecycle(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function is used to return the SECO FW build info.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    version     pointer to return build number
 * @param[out]    commit      pointer to return commit ID (git SHA-1)
 */
void sc_misc_seco_build_info(sc_ipc_t ipc, uint32_t *version,
    uint32_t *commit);

/*!
 * This function is used to return SECO chip info.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    lc          pointer to return lifecycle
 * @param[out]    monotonic   pointer to return monotonic counter
 * @param[out]    uid_l       pointer to return UID (lower 32 bits)
 * @param[out]    uid_h       pointer to return UID (upper 32 bits)
 */
sc_err_t sc_misc_seco_chip_info(sc_ipc_t ipc, uint16_t *lc,
    uint16_t *monotonic, uint32_t *uid_l, uint32_t *uid_h);

/*!
 * This function is used to return an event from the SECO error log.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    idx         index of event to return
 * @param[out]    event       pointer to return event
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Read of \a idx 0 captures events from SECO. Loop starting
 * with 0 until an error is returned to dump all events.
 */
sc_err_t sc_misc_seco_get_event(sc_ipc_t ipc, uint8_t idx,
    uint32_t *event);

/*!
 * This function is used to set the attestation mode. Only the owner of
 * the SC_R_ATTESTATION resource may make this call.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mode        mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if \a mode is invalid
 * - SC_ERR_NOACCESS if SC_R_ATTESTATON not owned by caller
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This is used to set the SECO attestation mode. This can be prover
 * or verfier. See the Security Reference Manual (SRM) for more on the
 * suported modes, mode values, and mode behavior.
 */
sc_err_t sc_misc_seco_attest_mode(sc_ipc_t ipc, uint32_t mode);

/*!
 * This function is used to request atestation. Only the owner of
 * the SC_R_ATTESTATION resource may make this call.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     nonce       unique value
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_NOACCESS if SC_R_ATTESTATON not owned by caller
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * This is used to ask SECO to perform an attestation. The result depends
 * on the attestation mode. After this call, the signature can be
 * requested or a verify can be requested.
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_attest(sc_ipc_t ipc, uint64_t nonce);

/*!
 * This function is used to retrieve the attestation public key.
 * Mode must be verifier. Only the owner of the SC_R_ATTESTATION resource
 * may make this call.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address to write response
 *
 * Result will be written to \a addr. The \a addr parmater must point
 * to an address SECO can access. It must be 64-bit aligned. There
 * should be 96 bytes of space.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if \a addr bad or attestation has not been requested
 * - SC_ERR_NOACCESS if SC_R_ATTESTATON not owned by caller
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_get_attest_pkey(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function is used to retrieve attestation signature and parameters.
 * Mode must be provider. Only the owner of the SC_R_ATTESTATION resource
 * may make this call.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address to write response
 *
 * Result will be written to \a addr. The \a addr parmater must point
 * to an address SECO can access. It must be 64-bit aligned. There
 * should be 120 bytes of space.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if \a addr bad or attestation has not been requested
 * - SC_ERR_NOACCESS if SC_R_ATTESTATON not owned by caller
 * - SC_ERR_UNAVAILABLE if SECO not available
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_get_attest_sign(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function is used to verify attestation. Mode must be verifier.
 * Only the owner of the SC_R_ATTESTATION resource may make this call.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     addr        address of signature
 *
 * The \a addr parmater must point to an address SECO can access. It must be
 * 64-bit aligned.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if \a addr bad or attestation has not been requested
 * - SC_ERR_NOACCESS if SC_R_ATTESTATON not owned by caller
 * - SC_ERR_UNAVAILABLE if SECO not available
 * - SC_ERR_FAIL if signature doesn't match
 *
 * See the Security Reference Manual (SRM) for more info.
 */
sc_err_t sc_misc_seco_attest_verify(sc_ipc_t ipc, sc_faddr_t addr);

/*!
 * This function is used to commit into the fuses any new SRK revocation
 * and FW version information that have been found in the primary and
 * secondary containers.
 *
 * @param[in]     ipc         IPC handle
 * @param[in,out] info        pointer to information type to be committed
 *
 * The return \a info will contain what was actually committed.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if \a info is invalid
 * - SC_ERR_UNAVAILABLE if SECO not available
 */
sc_err_t sc_misc_seco_commit(sc_ipc_t ipc, uint32_t *info);

/* @} */

/*!
 * @name Debug Functions
 * @{
 */

/*!
 * This function is used output a debug character from the SCU UART.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     ch          character to output
 */
void sc_misc_debug_out(sc_ipc_t ipc, uint8_t ch);

/*!
 * This function starts/stops emulation waveform capture.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     enable      flag to enable/disable capture
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_UNAVAILABLE if not running on emulation
 */
sc_err_t sc_misc_waveform_capture(sc_ipc_t ipc, sc_bool_t enable);

/*!
 * This function is used to return the SCFW build info.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    build       pointer to return build number
 * @param[out]    commit      pointer to return commit ID (git SHA-1)
 */
void sc_misc_build_info(sc_ipc_t ipc, uint32_t *build,
    uint32_t *commit);

/*!
 * This function is used to return the device's unique ID.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    id_l        pointer to return lower 32-bit of ID [31:0]
 * @param[out]    id_h        pointer to return upper 32-bits of ID [63:32]
 */
void sc_misc_unique_id(sc_ipc_t ipc, uint32_t *id_l,
    uint32_t *id_h);

/* @} */

/*!
 * @name Other Functions
 * @{
 */

/*!
 * This function configures the ARI match value for PCIe/SATA resources.
 *
 * @param[in]     ipc          IPC handle
 * @param[in]     resource     match resource
 * @param[in]     resource_mst PCIe/SATA master to match
 * @param[in]     ari          ARI to match
 * @param[in]     enable       enable match or not
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the owner or parent
 *   of the owner of the resource and translation
 *
 * For PCIe, the ARI is the 16-bit value that includes the bus number,
 * device number, and function number. For SATA, this value includes the
 * FISType and PM_Port.
 */
sc_err_t sc_misc_set_ari(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_rsrc_t resource_mst, uint16_t ari, sc_bool_t enable);

/*!
 * This function reports boot status.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     status      boot status
 *
 * This is used by SW partitions to report status of boot. This is
 * normally used to report a boot failure.
 */
void sc_misc_boot_status(sc_ipc_t ipc, sc_misc_boot_status_t status);

/*!
 * This function tells the SCFW that a CPU is done booting.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     cpu         CPU that is done booting
 *
 * This is called by early booting CPUs to report they are done with
 * initialization. After starting early CPUs, the SCFW halts the
 * booting process until they are done. During this time, early
 * CPUs can call the SCFW with lower latency as the SCFW is idle.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the CPU owner
 */
sc_err_t sc_misc_boot_done(sc_ipc_t ipc, sc_rsrc_t cpu);

/*!
 * This function reads a given fuse word index.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     word        fuse word index
 * @param[out]    val         fuse read value
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if word fuse index param out of range or invalid
 * - SC_ERR_NOACCESS if read operation failed
 * - SC_ERR_LOCKED if read operation is locked
 */
sc_err_t sc_misc_otp_fuse_read(sc_ipc_t ipc, uint32_t word, uint32_t *val);

/*!
 * This function writes a given fuse word index. Only the owner of the
 * SC_R_SYSTEM resource or a partition with access permissions to
 * SC_R_SYSTEM can do this.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     word        fuse word index
 * @param[in]     val         fuse write value
 *
 * The command is passed as is to SECO. SECO uses part of the
 * \a word parameter to indicate if the fuse should be locked
 * after programming. See the "Write common fuse" section of
 * the Security Reference Manual (SRM) for more info.
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if word fuse index param out of range or invalid
 * - SC_ERR_NOACCESS if caller does not have SC_R_SYSTEM access
 * - SC_ERR_NOACCESS if write operation failed
 * - SC_ERR_LOCKED if write operation is locked
 */
sc_err_t sc_misc_otp_fuse_write(sc_ipc_t ipc, uint32_t word, uint32_t val);

/*!
 * This function sets a temp sensor alarm.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource with sensor
 * @param[in]     temp        alarm to set
 * @param[in]     celsius     whole part of temp to set
 * @param[in]     tenths      fractional part of temp to set
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * This function will enable the alarm interrupt if the temp requested is
 * not the min/max temp. This enable automatically clears when the alarm
 * occurs and this function has to be called again to re-enable.
 *
 * Return errors codes:
 * - SC_ERR_PARM if parameters invalid
 * - SC_ERR_NOACCESS if caller does not own the resource
 * - SC_ERR_NOPOWER if power domain of resource not powered
 */
sc_err_t sc_misc_set_temp(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_misc_temp_t temp, int16_t celsius, int8_t tenths);

/*!
 * This function gets a temp sensor value.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource with sensor
 * @param[in]     temp        value to get (sensor or alarm)
 * @param[out]    celsius     whole part of temp to get
 * @param[out]    tenths      fractional part of temp to get
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors codes:
 * - SC_ERR_PARM if parameters invalid
 * - SC_ERR_BUSY if temp not ready yet (time delay after power on)
 * - SC_ERR_NOPOWER if power domain of resource not powered
 */
sc_err_t sc_misc_get_temp(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_misc_temp_t temp, int16_t *celsius, int8_t *tenths);

/*!
 * This function returns the boot device.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    dev         pointer to return boot device
 */
void sc_misc_get_boot_dev(sc_ipc_t ipc, sc_rsrc_t *dev);

/*!
 * This function returns the boot type.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    type        pointer to return boot type
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 *
 * Return errors code:
 * - SC_ERR_UNAVAILABLE if type not passed by ROM
 */
sc_err_t sc_misc_get_boot_type(sc_ipc_t ipc, sc_misc_bt_t *type);

/*!
 * This function returns the current status of the ON/OFF button.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    status      pointer to return button status
 */
void sc_misc_get_button_status(sc_ipc_t ipc, sc_bool_t *status);

/*!
 * This function returns the ROM patch checksum.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    checksum    pointer to return checksum
 *
 * @return Returns and error code (SC_ERR_NONE = success).
 */
sc_err_t sc_misc_rompatch_checksum(sc_ipc_t ipc, uint32_t *checksum);

/* @} */

#endif /* SC_MISC_API_H */

/**@}*/


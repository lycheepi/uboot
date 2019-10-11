/*
 * Copyright (C) 2018 iWave System Technologies Pvt Ltd.
 * Copyright 2018
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef IMX8QM_IWG27M_ANDROID_H
#define IMX8QM_IWG27M_ANDROID_H

#define CONFIG_USBD_HS
#define CONFIG_BOOTLOADER_OFFSET_33K

#define CONFIG_BCB_SUPPORT
#define CONFIG_CMD_READ
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_USB_GADGET_VBUS_DRAW	2

#define CONFIG_ANDROID_AB_SUPPORT
#define CONFIG_AVB_SUPPORT
#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_SYSTEM_RAMDISK_SUPPORT
#define CONFIG_AVB_FUSE_BANK_SIZEW 0
#define CONFIG_AVB_FUSE_BANK_START 0
#define CONFIG_AVB_FUSE_BANK_END 0
#define CONFIG_FASTBOOT_LOCK
#define FSL_FASTBOOT_FB_DEV "mmc"

#define IMX_LOAD_HDMI_FIMRWARE
#define IMX_HDMI_FIRMWARE_LOAD_ADDR (CONFIG_SYS_SDRAM_BASE + SZ_256M)
#define IMX_HDMI_FIRMWARE_SIZE 0x20000

#ifdef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN           (96 * SZ_1M)
#endif

#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT

#define CONFIG_ANDROID_BOOT_IMAGE
#define CONFIG_FASTBOOT_FLASH

#define CONFIG_FSL_FASTBOOT
#define CONFIG_ANDROID_RECOVERY

#if defined CONFIG_SYS_BOOT_SATA
#define CONFIG_FASTBOOT_STORAGE_SATA
#define CONFIG_FASTBOOT_SATA_NO 0
#else
#define CONFIG_FASTBOOT_STORAGE_SATA
#define CONFIG_FASTBOOT_SATA_NO 0
#define CONFIG_FASTBOOT_STORAGE_MMC
#endif

#define CONFIG_CMD_BOOTA
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SERIAL_TAG

#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"splashpos=m,m\0"	  \
	"fdt_high=0xffffffffffffffff\0"	  \
	"initrd_high=0xffffffffffffffff\0" \

#define CONFIG_FASTBOOT_BUF_ADDR   CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE   0x19000000

#endif /* IMX8QM_IWG27M_ANDROID_H */

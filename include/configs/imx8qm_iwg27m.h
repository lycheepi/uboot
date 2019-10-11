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

#ifndef __IMX8QM_IWG27M_ARM2_H
#define __IMX8QM_IWG27M_ARM2_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define BSP_VERSION		 	"iW-PRFHZ-SC-01-R2.0-REL1.0-Android8.1.0"

#define CONFIG_REMAKE_ELF

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ARCH_MISC_INIT

/* Flat Device Tree Definitions */
#define CONFIG_OF_BOARD_SETUP

#undef CONFIG_CMD_EXPORTENV
#undef CONFIG_CMD_IMPORTENV
#undef CONFIG_CMD_IMLS

#undef CONFIG_CMD_CRC32
#undef CONFIG_BOOTM_NETBSD

#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR       0
#define USDHC1_BASE_ADDR                0x5B010000
#define USDHC2_BASE_ADDR                0x5B020000
#define USDHC3_BASE_ADDR                0x5B030000
#define CONFIG_SUPPORT_EMMC_BOOT	/* eMMC specific */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SCSI
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID 1
#define CONFIG_CMD_SCSI
#define CONFIG_LIBATA
#define CONFIG_SYS_SCSI_MAX_LUN 1
#define CONFIG_SYS_SCSI_MAX_DEVICE      (CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_SYS_SCSI_MAXDEVICE       CONFIG_SYS_SCSI_MAX_DEVICE
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_SATA_IMX

#define CONFIG_FSL_HSIO
#define CONFIG_PCIE_IMX8X
#define CONFIG_CMD_PCI
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
/* FUSE command */
#define CONFIG_CMD_FUSE

/* GPIO configs */
#define CONFIG_MXC_GPIO

/* ENET Config */
#define CONFIG_MII

#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE             RGMII
#define FEC_QUIRK_ENET_MAC

#define CONFIG_PHY_GIGE /* Support for 1000BASE-X */
#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS

/* ENET0 connects AR8031 on CPU board, ENET1 connects to base board */
#define CONFIG_FEC_ENET_DEV 0 

#if (CONFIG_FEC_ENET_DEV == 0)
#define IMX_FEC_BASE			0x5B040000
#define CONFIG_FEC_MXC_PHYADDR          0x0
#define CONFIG_ETHPRIME                 "eth0"
#define CONFIG_FEC_MXC_MDIO_BASE	0x5B040000
#elif (CONFIG_FEC_ENET_DEV == 1)
#define IMX_FEC_BASE			0x5B050000
#define CONFIG_FEC_MXC_PHYADDR          0x0
#define CONFIG_ETHPRIME                 "eth1"
#define CONFIG_FEC_MXC_MDIO_BASE	0x5B050000
#endif

/* Boot M4 */
#define M4_BOOT_ENV \
	"m4_0_image=m4_0.bin\0" \
	"m4_1_image=m4_1.bin\0" \
	"loadm4image_0=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${m4_0_image}\0" \
	"loadm4image_1=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${m4_1_image}\0" \
	"m4boot_0=run loadm4image_0; dcache flush; bootaux ${loadaddr} 0\0" \
	"m4boot_1=run loadm4image_1; dcache flush; bootaux ${loadaddr} 1\0" \

#ifdef CONFIG_NAND_BOOT
#define MFG_NAND_PARTITION "mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),1m(misc),-(rootfs) "
#else
#define MFG_NAND_PARTITION ""
#endif

#ifdef CONFIG_MFG
#define CONFIG_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" "\
		"\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffff\0" \
	"bootcmd_mfg=run mfgtool_args;booti ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \

#define CONFIG_EXTRA_ENV_SETTINGS \
               CONFIG_MFG_ENV_SETTINGS \
               "fdt_addr=0x83000000\0" \
	       "fdt_high=0xffffffff\0" \
              "ethaddr=00:1a:ab:ff:ee:a1\0" \
               "eth1addr=1A:2B:3C:4D:5E:6F\0" \
               "\0" 

#else
/* Initial environment variables */

#define COMMON_CONFIG_EXTRA_ENV_SETTINGS \
        "fdt_file=fsl-imx8qm-iwg27m.dtb\0" \
        "fdt_addr=0x83000000\0" \
        "fdt_high=0xffffffff\0"   \
	"hdp_addr=0x84000000\0" \
        "hdp_file=hdmitxfw.bin\0" \
	"panel=NULL\0" \
	"disp=lcd\0" \
	"loadaddr=0x80280000\0"  \
               "ethaddr=00:01:02:03:04:05\0" \
				"eth1addr=1A:2B:3C:4D:5E:6F\0" \
                "kernel=boot-imx8qm.img\0" \
                "bootargs_base=console=ttyLP4,115200 earlycon=lpuart32,0x5a0a0000,115200,115200 init=/init androidboot.console=ttyLP4 " \
			"consoleblank=0 androidboot.hardware=freescale cma=800M androidboot.selinux=permissive\0" \
                "bootcmd_net=dhcp;run fdt_check;tftpboot ${loadaddr} ${serverip}:${kernel};tftpboot ${fdt_addr} ${serverip}:${fdt_file};" \
                        "run bootargs_net;booti ${loadaddr} - ${fdt_addr}\0" \
                "bootargs_net=setenv bootargs ${bootargs_base} " \
                        "root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \

#define CONFIG_EXTRA_ENV_SETTINGS               \
		COMMON_CONFIG_EXTRA_ENV_SETTINGS	\
	"bootargs_mmc=setenv bootargs ${bootargs_base} " \
                        "root=/dev/mmcblk0p2 rootwait rootfstype=ext4 rw \0" \
        "bootcmd_mmc=run bootargs_mmc;mmc dev 0;" \
                        "fatload mmc 0 ${loadaddr} ${kernel};fatload mmc 0 ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
        "bootcmd=boota mmc0\0" \
        "\0"
	
#define CONFIG_EXTRA_ENV_SETTINGS_SSD               \
                COMMON_CONFIG_EXTRA_ENV_SETTINGS        \
	"bootargs_ssd=setenv bootargs ${bootargs_base} " \
                     "root=/dev/mmcblk1p2 rootwait rootfstype=ext4 rw \0" \
        "bootcmd_ssd=run bootargs_ssd;mmc dev 1;" \
                        "fatload mmc 1 ${loadaddr} ${kernel};fatload mmc 1 ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
        "bootcmd=boota mmc1\0" \
	"\0"
	
#define CONFIG_EXTRA_ENV_SETTINGS_MSD               \
                COMMON_CONFIG_EXTRA_ENV_SETTINGS        \
         "bootargs_msd=setenv bootargs ${bootargs_base} " \
                        "root=/dev/mmcblk2p2 rootwait rootfstype=ext4 rw \0" \
         "bootcmd_msd=run bootargs_msd;mmc dev 2;" \
                        "fatload mmc 2 ${loadaddr} ${kernel};fatload mmc 2 ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
         "bootcmd=boota mmc2\0" \
        "\0"

#define CONFIG_EXTRA_ENV_SETTINGS_SATA               \
                COMMON_CONFIG_EXTRA_ENV_SETTINGS        \
        "bootargs_sata=setenv bootargs ${bootargs_base} " \
                        "root=/dev/sda2 rootwait rootfstype=ext4 rw \0" \
        "bootcmd_sata=run bootargs_sata;scsi scan;scsi dev 0;" \
                        "fatload scsi 0 ${loadaddr} ${kernel};fatload scsi 0 ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
        "bootcmd=boota sata0\0" \
        "\0"

#define CONFIG_EXTRA_ENV_SETTINGS_EMMC               \
                COMMON_CONFIG_EXTRA_ENV_SETTINGS        \
        "bootargs_mmc=setenv bootargs ${bootargs_base} " \
                        "root=/dev/mmcblk0p2 rootwait rootfstype=ext4 rw \0" \
        "bootcmd_mmc=run bootargs_mmc;mmc dev 0;" \
                        "fatload mmc 0 ${loadaddr} ${kernel};fatload mmc 0 ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
        "bootcmd=boota mmc0\0" \
        "\0"

#endif
	
/* Link Definitions */
#define CONFIG_LOADADDR			0x80280000
#define CONFIG_SYS_TEXT_BASE		0x80020000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_SYS_INIT_SP_ADDR         0x80200000


/* Default environment is in SD */
#define CONFIG_ENV_SIZE			0x1000

#ifdef CONFIG_QSPI_BOOT
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET       (4 * 1024 * 1024)
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SPI_BUS	CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS	CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE	CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#else
#define CONFIG_ENV_OFFSET       (64 * SZ_64K)
#define CONFIG_SYS_MMC_ENV_PART		0	/* user area */
#endif

#define CONFIG_SYS_MMC_IMG_LOAD_PART	1


#define CONFIG_FSL_ENV_IN_DYNAMIC
#ifndef CONFIG_MFG 
#if defined(CONFIG_FSL_ENV_IN_DYNAMIC)
/*dynamic environment variable setting*/

#define CONFIG_ENV_IS_IN_DYNAMIC
#define CONFIG_DYNAMIC_MMC_DEVNO
#define CONFIG_SYS_SATA_ENV_DEV		0
#else
#define CONFIG_ENV_IS_NOWHERE   1
#endif
#endif

#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_FSL_USDHC_NUM        2

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		((CONFIG_ENV_SIZE + (32*1024)) * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_NR_DRAM_BANKS		3
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
#define PHYS_SDRAM_1_SIZE		0x80000000	/* 2 GB */
#define PHYS_SDRAM_2_SIZE		0x100000000	/* 4 GB */

/*Memory test */
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START       0x80020000
#define CONFIG_SYS_MEMTEST_END         (CONFIG_SYS_MEMTEST_START+0x70000000)

/* Serial */
#ifdef CONFIG_SYS_PROMPT
#undef CONFIG_SYS_PROMPT
#endif
#define CONFIG_SYS_PROMPT              "iWave-G27 > "

#define CONFIG_BAUDRATE			115200

/* Monitor Command Prompt */
#define CONFIG_SYS_LONGHELP
#define CONFIG_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE              1024
#define CONFIG_SYS_MAXARGS             64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_CMDLINE_EDITING

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		8000000	/* 8MHz */

#define CONFIG_IMX_SMMU

/* MT35XU512ABA1G12 has only one Die, so QSPI0 B won't work */
#ifdef CONFIG_FSL_FSPI
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED	40000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define FSL_FSPI_FLASH_SIZE		SZ_64M
#define FSL_FSPI_FLASH_NUM		1
#define FSPI0_BASE_ADDR			0x5d120000
#define FSPI0_AMBA_BASE			0
#define CONFIG_SYS_FSL_FSPI_AHB
#endif

/* USB Config */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

/* USB 3.0 controller configs */
#ifdef CONFIG_USB_XHCI_IMX8
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	2
#endif

/* USB OTG controller configs */
#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif
#endif /* CONFIG_CMD_USB */

#ifdef CONFIG_USB_GADGET
#define CONFIG_USBD_HS
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#endif

#if defined(CONFIG_ANDROID_SUPPORT)
#include "imx8qm_iwg27m_android.h"
#endif

/* Framebuffer */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_IMXDPUV1
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_VIDEO_SKIP
#endif

#define CONFIG_OF_SYSTEM_SETUP
#define BOOTAUX_RESERVED_MEM_BASE 0x88000000
#define BOOTAUX_RESERVED_MEM_SIZE 0x08000000 /* Reserve from second 128MB */

#endif /* __IMX8QM_IWG27M_ARM2_H */

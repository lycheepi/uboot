/*
 * Copyright (C) 2018 iWave System Technologies Pvt Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMX8QM_IWG27M_MFG_ARM2_H
#define __IMX8QM_IWG27M_MFG_ARM2_H

#define CONFIG_CONSOLE_DEV              "ttyLP4"
#define CONFIG_MFG

#include "imx8qm_iwg27m.h"
#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#ifdef BSP_VERSION
#undef BSP_VERSION
#define BSP_VERSION                     "iW-PRFHZ-SC-01-R2.0-REL1.0-Linux4.9.51-MFG"
#endif

#define CONFIG_SYS_FSL_USDHC_NUM        2
#define CONFIG_SYS_MMC_ENV_DEV          1       /* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART         0       /* user partition */

/* USB Config */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

/* USB 3.0 controller configs */
#ifdef CONFIG_USB_XHCI_IMX8
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS      2
#endif

/* USB OTG controller configs */
#ifdef CONFIG_USB_EHCI_HCD

#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_FLAGS   0

#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC           (PORT_PTS_UTMI | PORT_PTS_PTW)
#endif
#endif /* CONFIG_CMD_USB */

#ifdef CONFIG_USB_GADGET
#define CONFIG_USBD_HS
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#endif

#ifdef CONFIG_ENV_IS_IN_DYNAMIC
#undef CONFIG_ENV_IS_IN_DYNAMIC
#endif
#define CONFIG_ENV_IS_NOWHERE 1

#ifdef CONFIG_USE_PLUGIN
#undef CONFIG_USE_PLUGIN
#endif

#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY 0

#ifdef CONFIG_BOOTARGS
#undef CONFIG_BOOTARGS
#endif
#define CONFIG_BOOTARGS "console=ttyLP4,115200 "\
                        "rdinit=/linuxrc "
#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif /* _IMX8QM_IWG27M_MFG_ARM2_H */
#define CONFIG_BOOTCOMMAND      "booti 0x80280000 0x83800000 0x83000000"

#endif

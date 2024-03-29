/*
 * Copyright 2017-2019 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/armv8/mmu.h>
#include <errno.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <fsl_wdog.h>
#include <imx_sip.h>
#include <generated/version_autogenerated.h>
#include <asm/setup.h>
#ifdef CONFIG_IMX_SEC_INIT
#include <fsl_caam.h>
#endif
#include <environment.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SECURE_BOOT) || defined(CONFIG_AVB_ATX) || defined(CONFIG_IMX_TRUSTY_OS)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 1,
	.word = 3,
};
#endif

int timer_init(void)
{
#ifdef CONFIG_SPL_BUILD
	struct sctr_regs *sctr = (struct sctr_regs *)SYSCNT_CTRL_BASE_ADDR;
	unsigned long freq = readl(&sctr->cntfid0);

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (freq) : "memory");

	clrsetbits_le32(&sctr->cntcr, SC_CNTCR_FREQ0 | SC_CNTCR_FREQ1,
			SC_CNTCR_FREQ0 | SC_CNTCR_ENABLE | SC_CNTCR_HDBG);
#endif

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}

void enable_tzc380(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable TZASC and lock setting */
	setbits_le32(&gpr->gpr[10], GPR_TZASC_EN);
	setbits_le32(&gpr->gpr[10], GPR_TZASC_EN_LOCK);
#if defined(CONFIG_IMX8MM) || defined(CONFIG_IMX8MN)
	setbits_le32(&gpr->gpr[10], GPR_TZASC_SWAP_ID);
#endif

	/*
	 * set Region 0 attribute to allow secure and non-secure read/write permission
	 * Found some masters like usb dwc3 controllers can't work with secure memory.
	 */
	writel(0xf0000000, TZASC_BASE_ADDR + 0x108);
}

void set_wdog_reset(struct wdog_regs *wdog)
{
	/*
	 * Output WDOG_B signal to reset external pmic or POR_B decided by
	 * the board design. Without external reset, the peripherals/DDR/
	 * PMIC are not reset, that may cause system working abnormal.
	 * WDZST bit is write-once only bit. Align this bit in kernel,
	 * otherwise kernel code will have no chance to set this bit.
	 */
	setbits_le16(&wdog->wcr, WDOG_WDT_MASK | WDOG_WDZST_MASK);
}

static struct mm_region imx8m_mem_map[] = {
	{
		/* ROM */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x100000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		.virt = 0x100000UL,
		.phys = 0x100000UL,
		.size = 0x8000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x7C0000UL,
		.phys = 0x7C0000UL,
		.size = 0x80000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* OCRAM */
		.virt = 0x900000UL,
		.phys = 0x900000UL,
		.size = 0x200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* AIPS */
		.virt = 0xB00000UL,
		.phys = 0xB00000UL,
		.size = 0x3f500000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM1 */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
#ifdef CONFIG_IMX_TRUSTY_OS
			 PTE_BLOCK_INNER_SHARE
#else
			 PTE_BLOCK_OUTER_SHARE
#endif
#if CONFIG_NR_DRAM_BANKS > 1
	}, {
		/* DRAM2 */
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = PHYS_SDRAM_2_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
#ifdef CONFIG_IMX_TRUSTY_OS
			 PTE_BLOCK_INNER_SHARE
#else
			 PTE_BLOCK_OUTER_SHARE
#endif
#endif
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = imx8m_mem_map;

void enable_caches(void)
{
	/* If OPTEE runs, remove OPTEE memory from MMU table to avoid speculative prefetch */
	if (rom_pointer[1]) {
		imx8m_mem_map[5].size -= rom_pointer[1];
	}

	icache_enable();
	dcache_enable();
}

static u32 get_cpu_variant_type(u32 type)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->tester4);

	if (type == MXC_CPU_IMX8MQ) {
		if ((value & 0x3) == 0x2)
			return MXC_CPU_IMX8MD;
		else if (value & 0x200000)
			return MXC_CPU_IMX8MQL;

	} else if (type == MXC_CPU_IMX8MM) {
		switch (value & 0x3) {
		case 2:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MMDL;
			else
				return MXC_CPU_IMX8MMD;
		case 3:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MMSL;
			else
				return MXC_CPU_IMX8MMS;
		default:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MML;
			break;
		}
	} else if (type == MXC_CPU_IMX8MN) {
		switch (value & 0x3) {
		case 2:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNDL;
			else
				return MXC_CPU_IMX8MND;
		case 3:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNSL;
			else
				return MXC_CPU_IMX8MNS;
		default:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNL;
			break;
		}
	}

	return type;
}

u32 get_cpu_rev(void)
{
	struct anamix_pll *ana_pll = (struct anamix_pll *)ANATOP_BASE_ADDR;
	u32 reg = readl(&ana_pll->digprog);
	u32 type = (reg >> 16) & 0xff;
	u32 major_low = (reg >> 8) & 0xff;
	u32 rom_version;

	reg &= 0xff;

	/* iMX8MN */
	 if (major_low == 0x42) {
		type = get_cpu_variant_type(MXC_CPU_IMX8MN);
		return (type << 12) | reg;
	 } else if (major_low == 0x41) {
		/* iMX8MM */
		type = get_cpu_variant_type(MXC_CPU_IMX8MM);
		return (type << 12) | reg;
	} else {
		/* iMX8MQ */
		if (reg == CHIP_REV_1_0) {
			/*
			 * For B0 chip, the DIGPROG is not updated, still TO1.0.
			 * we have to check ROM version or OCOTP_READ_FUSE_DATA
			 */
			if (readl((void __iomem *)(OCOTP_BASE_ADDR + 0x40))
				== 0xff0055aa) {
				/* 0xff0055aa is magic number for B1 */
				reg = CHIP_REV_2_1;
			} else {
				rom_version = readb((void __iomem *)ROM_VERSION_A0);
				if (rom_version != CHIP_REV_1_0) {
					rom_version = readb((void __iomem *)ROM_VERSION_B0);
					if (rom_version == CHIP_REV_2_0)
						reg = CHIP_REV_2_0;
				}
			}
		}

		type = get_cpu_variant_type(type);

		return (type << 12) | reg;
	}
}

static void imx_set_wdog_powerdown(bool enable)
{
	struct wdog_regs *wdog1 = (struct wdog_regs *)WDOG1_BASE_ADDR;
	struct wdog_regs *wdog2 = (struct wdog_regs *)WDOG2_BASE_ADDR;
	struct wdog_regs *wdog3 = (struct wdog_regs *)WDOG3_BASE_ADDR;

	/* Write to the PDE (Power Down Enable) bit */
	writew(enable, &wdog1->wmcr);
	writew(enable, &wdog2->wmcr);
	writew(enable, &wdog3->wmcr);
}

int arch_cpu_init(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	/*
	 * Init timer at very early state, because pll setting will use it,
	 * Rom Turnned off SCTR, enable it before timer_init
	 */

	clock_enable(CCGR_SCTR, 1);
	timer_init();

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		clock_init();
		imx_set_wdog_powerdown(false);

		if (is_imx8md() || is_imx8mmd() || is_imx8mmdl() || is_imx8mms() || is_imx8mmsl() ||
			is_imx8mnd() || is_imx8mndl() || is_imx8mns() || is_imx8mnsl()) {
			/* Power down cpu core 1, 2 and 3 for iMX8M Dual core or Single core */
			struct pgc_reg *pgc_core1 = (struct pgc_reg *)(GPC_BASE_ADDR + 0x840);
			struct pgc_reg *pgc_core2 = (struct pgc_reg *)(GPC_BASE_ADDR + 0x880);
			struct pgc_reg *pgc_core3 = (struct pgc_reg *)(GPC_BASE_ADDR + 0x8C0);
			struct gpc_reg *gpc = (struct gpc_reg *)GPC_BASE_ADDR;

			writel(0x1, &pgc_core2->pgcr);
			writel(0x1, &pgc_core3->pgcr);
			if (is_imx8mms() || is_imx8mmsl() || is_imx8mns() || is_imx8mnsl()) {
				writel(0x1, &pgc_core1->pgcr);
				writel(0xE, &gpc->cpu_pgc_dn_trg);
			} else {
				writel(0xC, &gpc->cpu_pgc_dn_trg);
			}
		}
	}

#ifdef CONFIG_IMX_SEC_INIT
	/* Secure init function such RNG */
	imx_sec_init();
#endif
#if defined(CONFIG_ANDROID_SUPPORT)
	/* Enable RTC */
	writel(0x21, 0x30370038);
#endif

	if (is_imx8mq()) {
		clock_enable(CCGR_OCOTP, 1);
		if (readl(&ocotp->ctrl) & 0x200)
			writel(0x200, &ocotp->ctrl_clr);
	}

	return 0;
}

#ifdef CONFIG_IMX8MN
struct rom_api
{
	uint16_t ver;
	uint16_t tag;
	uint32_t reserved1;
	uint32_t (*download_image)(uint8_t * dest, uint32_t offset, uint32_t size,  uint32_t xor);
	uint32_t (*query_boot_infor)(uint32_t info_type, uint32_t *info, uint32_t xor);
};
static struct rom_api* g_rom_api = (struct rom_api*)0x980;

typedef enum {
    BT_DEV_TYPE_SD = 1,
    BT_DEV_TYPE_MMC = 2,
    BT_DEV_TYPE_NAND = 3,
    BT_DEV_TYPE_FLEXSPINOR = 4,

    BT_DEV_TYPE_USB = 0xE,
    BT_DEV_TYPE_MEM_DEV = 0xF,

    BT_DEV_TYPE_INVALID = 0xFF
} boot_dev_type_e;

#define QUERY_BT_DEV		2

#define ROM_API_OKAY		0xF0

enum boot_device get_boot_device(void)
{
	volatile gd_t *pgd = gd;
	int ret;
	uint32_t boot;
	u16 boot_type;
	u8 boot_instance;
	enum boot_device boot_dev = SD1_BOOT;

	ret = g_rom_api->query_boot_infor(QUERY_BT_DEV, &boot, ((uintptr_t) &boot)^ QUERY_BT_DEV);
        gd =  pgd;

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: failure at query_boot_info\n");
		return -1;
	}

	boot_type = boot >> 16;
	boot_instance = (boot >> 8) & 0xff;

	switch (boot_type) {
	case BT_DEV_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BT_DEV_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BT_DEV_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BT_DEV_TYPE_FLEXSPINOR:
		boot_dev = QSPI_BOOT;
		break;
	case BT_DEV_TYPE_USB:
		boot_dev = USB_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}
#endif

bool is_usb_boot(void)
{
	return get_boot_device() == USB_BOOT;
}
#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;

	serialnr->low = fuse->uid_low;
	serialnr->high = fuse->uid_high;
}
#endif

#ifdef CONFIG_OF_SYSTEM_SETUP
static int ft_add_optee_node(void *fdt, bd_t *bd)
{
	const char *path, *subpath;
	int offs;

	/*
	 * No TEE space allocated indicating no TEE running, so no
	 * need to add optee node in dts
	 */
	if (!rom_pointer[1])
		return 0;

	offs = fdt_increase_size(fdt, 512);
	if (offs) {
		printf("No Space for dtb\n");
		return 1;
	}

	path = "/firmware";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		path = "/";
		offs = fdt_path_offset(fdt, path);

		if (offs < 0) {
			printf("Could not find root node.\n");
			return 1;
		}

		subpath = "firmware";
		offs = fdt_add_subnode(fdt, offs, subpath);
		if (offs < 0) {
			printf("Could not create %s node.\n", subpath);
		}
	}

	subpath = "optee";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s node.\n", subpath);
	}

	fdt_setprop_string(fdt, offs, "compatible", "linaro,optee-tz");
	fdt_setprop_string(fdt, offs, "method", "smc");

	return 0;
}

static int disable_fdt_nodes(void *blob, const char *nodes_path[], int size_array)
{
	int i = 0;
	int rc;
	int nodeoff;
	const char *status = "disabled";

	for (i = 0; i < size_array; i++) {
		nodeoff = fdt_path_offset(blob, nodes_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		printf("Found %s node\n", nodes_path[i]);

add_status:
		rc = fdt_setprop(blob, nodeoff, "status", status, strlen(status) + 1);
		if (rc) {
			if (rc == -FDT_ERR_NOSPACE) {
				rc = fdt_increase_size(blob, 512);
				if (!rc)
					goto add_status;
			}
			printf("Unable to update property %s:%s, err=%s\n",
				nodes_path[i], "status", fdt_strerror(rc));
		} else {
			printf("Modify %s:%s disabled\n",
				nodes_path[i], "status");
		}
	}

	return 0;
}

#ifdef CONFIG_IMX8MQ
bool check_dcss_fused(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->tester4);
	if (value & 0x4000000)
		return true;

	return false;
}

static int disable_mipi_dsi_nodes(void *blob)
{
	const char *nodes_path[] = {
		"/mipi_dsi@30A00000",
		"/mipi_dsi_bridge@30A00000",
		"/dsi_phy@30A00300"
	};

	return disable_fdt_nodes(blob, nodes_path, ARRAY_SIZE(nodes_path));
}

static int disable_dcss_nodes(void *blob)
{
	const char *nodes_path[] = {
		"/dcss@0x32e00000",
		"/dcss@32e00000",
		"/hdmi@32c00000",
		"/hdmi_cec@32c33800",
		"/hdmi_drm@32c00000",
		"/display-subsystem",
		"/sound-hdmi",
		"/sound-hdmi-arc"
	};

	return disable_fdt_nodes(blob, nodes_path, ARRAY_SIZE(nodes_path));
}

static int check_mipi_dsi_nodes(void *blob)
{
	const char *lcdif_path = "/lcdif@30320000";
	const char *mipi_dsi_path = "/mipi_dsi@30A00000";

	const char *lcdif_ep_path = "/lcdif@30320000/port@0/mipi-dsi-endpoint";
	const char *mipi_dsi_ep_path = "/mipi_dsi@30A00000/port@1/endpoint";

	int nodeoff;
	nodeoff = fdt_path_offset(blob, lcdif_path);
	if (nodeoff < 0 || !fdtdec_get_is_enabled(blob, nodeoff)) {
		/* If can't find lcdif node or lcdif node is disabled, then disable all mipi dsi,
		    since they only can input from DCSS */
		return disable_mipi_dsi_nodes(blob);
	}

	nodeoff = fdt_path_offset(blob, mipi_dsi_path);
	if (nodeoff < 0 || !fdtdec_get_is_enabled(blob, nodeoff))
		return 0;

	nodeoff = fdt_path_offset(blob, lcdif_ep_path);
	if (nodeoff < 0) {
		/* If can't find lcdif endpoint, then disable all mipi dsi,
		    since they only can input from DCSS */
		return disable_mipi_dsi_nodes(blob);
	} else {
		int lookup_node;
		lookup_node = fdtdec_lookup_phandle(blob, nodeoff, "remote-endpoint");
		nodeoff = fdt_path_offset(blob, mipi_dsi_ep_path);

		if (nodeoff >0 && nodeoff == lookup_node)
			return 0;

		return disable_mipi_dsi_nodes(blob);
	}

}

void board_quiesce_devices(void)
{
#ifdef CONFIG_USB_DWC3
	if (is_usb_boot())
		disconnect_from_pc();
#endif
}
#endif

int disable_vpu_nodes(void *blob)
{
	const char *nodes_path_8mq[] = {
		"/vpu@38300000"
	};

	const char *nodes_path_8mm[] = {
		"/vpu_g1@38300000",
		"/vpu_g2@38310000",
		"/vpu_h1@38320000"
	};

	if (is_imx8mq())
		return disable_fdt_nodes(blob, nodes_path_8mq, ARRAY_SIZE(nodes_path_8mq));
	else if (is_imx8mm())
		return disable_fdt_nodes(blob, nodes_path_8mm, ARRAY_SIZE(nodes_path_8mm));
	else
		return -EPERM;

}

int disable_gpu_nodes(void *blob)
{
	const char *nodes_path_8mn[] = {
		"/gpu@38000000"
	};

	return disable_fdt_nodes(blob, nodes_path_8mn, ARRAY_SIZE(nodes_path_8mn));
}

int disable_cpu_nodes(void *blob, u32 disabled_cores)
{
	const char *nodes_path[] = {
			"/cpus/cpu@1",
			"/cpus/cpu@2",
			"/cpus/cpu@3",
	};

	u32 i = 0;
	int rc;
	int nodeoff;

	if (disabled_cores > 3)
		return -EINVAL;

	i = 3 - disabled_cores;

	for (; i < 3; i++) {
		nodeoff = fdt_path_offset(blob, nodes_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		printf("Found %s node\n", nodes_path[i]);

		rc = fdt_del_node(blob, nodeoff);
		if (rc < 0) {
			printf("Unable to delete node %s, err=%s\n",
				nodes_path[i], fdt_strerror(rc));
		} else {
			printf("Delete node %s\n", nodes_path[i]);
		}
	}

	return 0;
}

int ft_system_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_IMX8MQ
	int i = 0;
	int rc;
	int nodeoff;

	if (get_boot_device() == USB_BOOT) {

		disable_dcss_nodes(blob);

		const char *usb_dwc3_path = "/usb@38100000/dwc3";
		nodeoff = fdt_path_offset(blob, usb_dwc3_path);
		if (nodeoff >= 0) {
			const char *speed = "high-speed";
			printf("Found %s node\n", usb_dwc3_path);

usb_modify_speed:

			rc = fdt_setprop(blob, nodeoff, "maximum-speed", speed, strlen(speed) + 1);
			if (rc) {
				if (rc == -FDT_ERR_NOSPACE) {
					rc = fdt_increase_size(blob, 512);
					if (!rc)
						goto usb_modify_speed;
				}
				printf("Unable to set property %s:%s, err=%s\n",
					usb_dwc3_path, "maximum-speed", fdt_strerror(rc));
			} else {
				printf("Modify %s:%s = %s\n",
					usb_dwc3_path, "maximum-speed", speed);
			}
		}else {
			printf("Can't found %s node\n", usb_dwc3_path);
		}
	}

	/* Disable the CPU idle for A0 chip since the HW does not support it */
	if (is_soc_rev(CHIP_REV_1_0)) {
		static const char * const nodes_path[] = {
			"/cpus/cpu@0",
			"/cpus/cpu@1",
			"/cpus/cpu@2",
			"/cpus/cpu@3",
		};

		for (i = 0; i < ARRAY_SIZE(nodes_path); i++) {
			nodeoff = fdt_path_offset(blob, nodes_path[i]);
			if (nodeoff < 0)
				continue; /* Not found, skip it */

			printf("Found %s node\n", nodes_path[i]);

			rc = fdt_delprop(blob, nodeoff, "cpu-idle-states");
			if (rc) {
				printf("Unable to update property %s:%s, err=%s\n",
				       nodes_path[i], "status", fdt_strerror(rc));
				return rc;
			}

			printf("Remove %s:%s\n", nodes_path[i],
			       "cpu-idle-states");
		}
	}

	if (is_imx8mql()) {
		disable_vpu_nodes(blob);
		if (check_dcss_fused()) {
			printf("DCSS is fused\n");
			disable_dcss_nodes(blob);
			check_mipi_dsi_nodes(blob);
		}
	}

	if (is_imx8md())
		disable_cpu_nodes(blob, 2);

#elif defined(CONFIG_IMX8MM)
	if (is_imx8mml() || is_imx8mmdl() ||  is_imx8mmsl())
		disable_vpu_nodes(blob);

	if (is_imx8mmd() || is_imx8mmdl())
		disable_cpu_nodes(blob, 2);
	else if (is_imx8mms() || is_imx8mmsl())
		disable_cpu_nodes(blob, 3);

#elif defined(CONFIG_IMX8MN)
	if (is_imx8mnl() || is_imx8mndl() ||  is_imx8mnsl())
		disable_gpu_nodes(blob);

	if (is_imx8mnd() || is_imx8mndl())
		disable_cpu_nodes(blob, 2);
	else if (is_imx8mns() || is_imx8mnsl())
		disable_cpu_nodes(blob, 3);

#ifdef CONFIG_IMX8MN_FORCE_NOM_SOC
	/* Disable the DVFS by removing 1.4Ghz and 1.5Ghz operating-points*/
	int rc;
	int nodeoff;
	static const char * const nodes_path = "/cpus/cpu@0";
	u32 val[] = {1200000, 850000};

	nodeoff = fdt_path_offset(blob, nodes_path);
	if (nodeoff < 0) {
		printf("Unable to find node %s, err=%s\n",
		       nodes_path, fdt_strerror(nodeoff));
		return nodeoff;
	}

	printf("Found %s node\n", nodes_path);

	val[0] = cpu_to_fdt32(val[0]);
	val[1] = cpu_to_fdt32(val[1]);
	rc = fdt_setprop(blob, nodeoff, "operating-points", &val, 2 * sizeof(u32));
	if (rc) {
		printf("Unable to update operating-points for node %s, err=%s\n",
		       nodes_path, fdt_strerror(rc));
		return rc;
	}

	printf("Update %s:%s\n", nodes_path,
	       "operating-points");

#endif /* CONFIG_IMX8MN_FORCE_NOM_SOC */

#endif

	return ft_add_optee_node(blob, bd);
}
#endif

void reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	/* Clear WDA to trigger WDOG_B immediately */
	writew((WCR_WDE | WCR_SRS), &wdog->wcr);

	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}

#if defined(CONFIG_ARCH_MISC_INIT)
#define FSL_SIP_BUILDINFO			0xC2000003
#define FSL_SIP_BUILDINFO_GET_COMMITHASH	0x00
static void acquire_buildinfo(void)
{
	uint64_t atf_commit = 0;

	/* Get ARM Trusted Firmware commit id */
	atf_commit = call_imx_sip(FSL_SIP_BUILDINFO,
				  FSL_SIP_BUILDINFO_GET_COMMITHASH, 0, 0, 0);
	if (atf_commit == 0xffffffff) {
		debug("ATF does not support build info\n");
		atf_commit = 0x30; /* Display 0, 0 ascii is 0x30 */
	}

	printf("\n BuildInfo:\n  - ATF %s\n  - %s\n\n", (char *)&atf_commit,
	       U_BOOT_VERSION);
}

int arch_misc_init(void)
{
	acquire_buildinfo();

	return 0;
}
#endif

#define FSL_SIP_GPC			0xC2000000
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x03

#ifdef CONFIG_SPL_BUILD
static uint32_t gpc_pu_m_core_offset[11] = {
	0xc00, 0xc40, 0xc80, 0xcc0,
	0xdc0, 0xe00, 0xe40, 0xe80,
	0xec0, 0xf00, 0xf40,
};

#define PGC_PCR				0

void imx_gpc_set_m_core_pgc(unsigned int offset, bool pdn)
{
	uint32_t val;
	uintptr_t reg = GPC_BASE_ADDR + offset;

	val = readl(reg);
	val &= ~(0x1 << PGC_PCR);

	if(pdn)
		val |= 0x1 << PGC_PCR;
	writel(val, reg);
}

void imx8m_usb_power_domain(uint32_t domain_id, bool on)
{
	uint32_t val;
	uintptr_t reg;

	imx_gpc_set_m_core_pgc(gpc_pu_m_core_offset[domain_id], true);

	reg = GPC_BASE_ADDR + (on ? 0xf8 : 0x104);
	val = 1 << (domain_id > 3 ? (domain_id + 3) : domain_id);
	writel(val, reg);
	while (readl(reg) & val)
		;
	imx_gpc_set_m_core_pgc(gpc_pu_m_core_offset[domain_id], false);
}
#endif

int imx8m_usb_power(int usb_id, bool on)
{
	if (usb_id > 1)
		return -EINVAL;
#ifdef CONFIG_SPL_BUILD
	imx8m_usb_power_domain(2 + usb_id, on);
#else
	if (call_imx_sip(FSL_SIP_GPC, FSL_SIP_CONFIG_GPC_PM_DOMAIN,
			 2 + usb_id, on, 0))
		return -EPERM;
#endif
	return 0;
}

void nxp_tmu_arch_init(void *reg_base)
{
	if (is_imx8mm()) {
		/* Load TCALIV and TASR from fuses */
		struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
		struct fuse_bank *bank = &ocotp->bank[3];
		struct fuse_bank3_regs *fuse =
			(struct fuse_bank3_regs *)bank->fuse_regs;

		u32 tca_rt, tca_hr, tca_en;
		u32 buf_vref, buf_slope;

		tca_rt = fuse->ana0 & 0xFF;
		tca_hr = (fuse->ana0 & 0xFF00) >> 8;
		tca_en = (fuse->ana0 & 0x2000000) >> 25;

		buf_vref = (fuse->ana0 & 0x1F00000) >> 20;
		buf_slope = (fuse->ana0 & 0xF0000) >> 16;

		writel(buf_vref | (buf_slope << 16), (ulong)reg_base + 0x28);
		writel((tca_en << 31) |(tca_hr <<16) | tca_rt,  (ulong)reg_base + 0x30);
	}
}

#if defined(CONFIG_IMX8MN)
enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_device dev = get_boot_device();
	enum env_location env_loc = ENVL_UNKNOWN;

	if (prio)
		return env_loc;

	switch (dev) {
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	case QSPI_BOOT:
		env_loc = ENVL_SPI_FLASH;
		break;
#endif
#ifdef CONFIG_ENV_IS_IN_NAND
	case NAND_BOOT:
		env_loc = ENVL_NAND;
		break;
#endif
#ifdef CONFIG_ENV_IS_IN_MMC
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		env_loc =  ENVL_MMC;
		break;
#endif
	default:
#ifdef CONFIG_ENV_DEFAULT_NOWHERE
		env_loc = ENVL_NOWHERE;
#endif
		break;
	}

	return env_loc;
}

#ifndef ENV_IS_EMBEDDED
long long env_get_offset(long long defautl_offset)
{
	enum boot_device dev = get_boot_device();

	switch (dev) {
	case NAND_BOOT:
		return (60 << 20);  /* 60MB offset for NAND */
	default:
		break;
	}

	return defautl_offset;
}
#endif
#endif

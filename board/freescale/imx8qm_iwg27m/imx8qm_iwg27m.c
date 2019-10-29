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
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include "pca953x.h"

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <dm.h>
#include <imx8_hsio.h>
#include <usb.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/video.h>
#include <asm/arch/video_common.h>
#include <power-domain.h>

DECLARE_GLOBAL_DATA_PTR;

#define ESDHC_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ESDHC_CLK_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))


#define ENET_INPUT_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define FSPI_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CFG_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_NONE << PADRING_PULL_SHIFT))

#define I2C_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart4_pads[] = {
	SC_P_M40_GPIO0_00 | MUX_MODE_ALT(2) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_M40_GPIO0_01 | MUX_MODE_ALT(2) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

#define LCD_RST IMX_GPIO_NR(0, 3)
static iomux_cfg_t lcd_rst_pads[] = {
        SC_P_SIM0_PD | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void lcd_reset(void)
{
        imx8_iomux_setup_multiple_pads(lcd_rst_pads, ARRAY_SIZE(lcd_rst_pads));
        gpio_request(LCD_RST, "LCD-GPIO");
        gpio_direction_output(LCD_RST,1);
	mdelay(20);
        gpio_direction_output(LCD_RST,0);
}

int board_early_init_f(void)
{
	sc_ipc_t ipcHndl = 0;
	sc_err_t sciErr = 0;

	ipcHndl = gd->arch.ipc_channel_handle;

	/* Power up UART4, this is very early while power domain is not working */
	sciErr = sc_pm_set_resource_power_mode(ipcHndl, SC_R_UART_4, SC_PM_PW_MODE_ON);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Set UART4 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_UART_4, 2, &rate);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Enable UART4 clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_UART_4, 2, true, false);
	if (sciErr != SC_ERR_NONE)
		return 0;

	setup_iomux_uart();

	return 0;
}

#ifdef CONFIG_FSL_ESDHC

#define USDHC1_CD_GPIO	IMX_GPIO_NR(1, 23)
#define USDHC1_WP_GPIO	IMX_GPIO_NR(1, 22)
#define USDHC1_PWR_EN_GPIO	IMX_GPIO_NR(1, 19)

static struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
	{USDHC1_BASE_ADDR, 0, 8}, /* eMMC*/
	{USDHC2_BASE_ADDR, 0, 4}, /* SSD*/
};

/* eMMC */
static iomux_cfg_t emmc0[] = {
	SC_P_EMMC0_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_EMMC0_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA4 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA5 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA6 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA7 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_STROBE | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

/* SSD */
static iomux_cfg_t usdhc1_sd[] = {
	SC_P_USDHC1_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_USDHC1_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_MIPI_DSI1_GPIO0_00 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL), /* Mux for WP, GPIO01 IO22 */
	SC_P_MIPI_DSI1_GPIO0_01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL), /* Mux for CD,  GPIO1 IO23 */
	SC_P_MIPI_DSI0_GPIO0_01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL), /* POWER ENABLE,  GPIO1 IO19 */
};

int board_mmc_init(bd_t *bis)
{
	int i, ret;
	struct power_domain pd;

	printf("HUYTV12: board_mmc_init ok!\n");

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1--eMMC
	 * mmc1                    USDHC2--SSD
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			if (!power_domain_lookup_name("conn_sdhc0", &pd))
				power_domain_on(&pd);

			imx8_iomux_setup_multiple_pads(emmc0, ARRAY_SIZE(emmc0));
			init_clk_usdhc(0);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			if (!power_domain_lookup_name("conn_sdhc1", &pd))
				power_domain_on(&pd);

			imx8_iomux_setup_multiple_pads(usdhc1_sd, ARRAY_SIZE(usdhc1_sd));
			init_clk_usdhc(1);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			gpio_request(USDHC1_CD_GPIO, "sd1_cd");
			gpio_direction_input(USDHC1_CD_GPIO);
			gpio_request(USDHC1_WP_GPIO, "sd1_wp");
			gpio_direction_input(USDHC1_WP_GPIO);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return 0;
		}
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1; /* eMMC */
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO); /*SSD*/
		break;
	}

	return ret;
}

#endif /* CONFIG_FSL_ESDHC */


#ifdef CONFIG_FEC_MXC
#include <miiphy.h>

static iomux_cfg_t pad_enet1[] = {
	SC_P_ENET1_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_RXD0 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_RXD1 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_RXD2 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_RXD3 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_RXC | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET1_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_RGMII_TXD0 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_RGMII_TXD1 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_RGMII_TXD2 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_RGMII_TXD3 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_RGMII_TXC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_REFCLK_125M_25M | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET1_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_MIPI_DSI1_I2C0_SCL | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
        SC_P_MIPI_DSI0_I2C0_SCL | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
        SC_P_MIPI_DSI0_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static iomux_cfg_t pad_enet0[] = {
	SC_P_ENET0_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD0 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD1 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD2 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD3 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXC | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD0 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD1 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD2 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD3 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_REFCLK_125M_25M | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
        SC_P_PCIE_CTRL1_WAKE_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
        SC_P_PCIE_CTRL1_CLKREQ_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
        SC_P_PCIE_CTRL0_CLKREQ_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	if (0 == CONFIG_FEC_ENET_DEV)
		imx8_iomux_setup_multiple_pads(pad_enet0, ARRAY_SIZE(pad_enet0));
	else
		imx8_iomux_setup_multiple_pads(pad_enet1, ARRAY_SIZE(pad_enet1));
}

static void enet_device_phy_reset(void)
{
	struct gpio_desc desc;
	int ret;

	if (0 == CONFIG_FEC_ENET_DEV) {
		ret = dm_gpio_lookup_name("gpio4_31", &desc);
		if (ret)
			return;

		ret = dm_gpio_request(&desc, "enet0_reset");
		if (ret)
			return;
	} else {
		ret = dm_gpio_lookup_name("gpio1_20", &desc);
		if (ret)
			return;

		ret = dm_gpio_request(&desc, "enet1_reset");
		if (ret)
			return;
	}

	dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	dm_gpio_set_value(&desc, 0);
	udelay(50);
	dm_gpio_set_value(&desc, 1);

	/* The board has a long delay for this reset to become stable */
	mdelay(200);
}

int board_eth_init(bd_t *bis)
{
	int ret;
	struct power_domain pd;

	printf("[%s] %d\n", __func__, __LINE__);

	if (CONFIG_FEC_ENET_DEV) {
		if (!power_domain_lookup_name("conn_enet1", &pd))
			power_domain_on(&pd);
	} else {
		if (!power_domain_lookup_name("conn_enet0", &pd))
			power_domain_on(&pd);
	}

	setup_iomux_fec();

	ret = fecmxc_initialize_multi(bis, CONFIG_FEC_ENET_DEV,
		CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
	if (ret)
		printf("FEC1 MXC: %s:failed\n", __func__);

	return ret;
}

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static int setup_fec(int ind)
{
	/* Reset ENET PHY */
	enet_device_phy_reset();

	return 0;
}
#endif

#define BCONFIG_0 IMX_GPIO_NR(1, 5)
#define BCONFIG_1 IMX_GPIO_NR(1, 13)
#define BCONFIG_2 IMX_GPIO_NR(1, 12)
#define BCONFIG_3 IMX_GPIO_NR(1, 11)
#define BCONFIG_4 IMX_GPIO_NR(0, 6)
#define BCONFIG_5 IMX_GPIO_NR(0, 7)
#define BCONFIG_6 IMX_GPIO_NR(0, 11)

int board_config_pads[] = {
	BCONFIG_0,
	BCONFIG_1,
	BCONFIG_2,
	BCONFIG_3,
	BCONFIG_4,	
	BCONFIG_5,
	BCONFIG_6,
};

static iomux_cfg_t board_cfg[] = {
	SC_P_LVDS0_GPIO01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_LVDS1_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_LVDS1_I2C0_SCL | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_LVDS1_GPIO01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_M40_I2C0_SCL | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_M40_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
	SC_P_M41_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
};

#define OTG_PWR IMX_GPIO_NR(4, 3)
static iomux_cfg_t otg_pwr_pads[] = {
        SC_P_USB_SS3_TC0 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void usb_otg_pwr_enable(void)
{
         imx8_iomux_setup_multiple_pads(otg_pwr_pads, ARRAY_SIZE(otg_pwr_pads));
        gpio_request(OTG_PWR, "OTG-GPIO");
         gpio_direction_output(OTG_PWR,1);
}

#define HUB_PWR IMX_GPIO_NR(4, 7)
#define HUB_RST IMX_GPIO_NR(1, 21)

static iomux_cfg_t hub_pwr_pads[] = {
	SC_P_USDHC1_RESET_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
	SC_P_MIPI_DSI1_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void usb_hub_pwr_enable(void)
{
	imx8_iomux_setup_multiple_pads(hub_pwr_pads, ARRAY_SIZE(hub_pwr_pads));
	gpio_request(HUB_PWR, "usb_hub_power_gpio");
	gpio_direction_output(HUB_PWR,1);
	udelay(50);
	gpio_request(HUB_RST, "usb_hub_reset_gpio");
	gpio_direction_output(HUB_RST,1);
}

static void printboard_info(void)
{
	int i, bom_rev, pcb_rev;

	imx8_iomux_setup_multiple_pads(board_cfg, ARRAY_SIZE(board_cfg));	

	for (i=0;i<ARRAY_SIZE(board_config_pads);i++)
	{	
		if(i<=3)
		{
			gpio_request(board_config_pads[i], "SOM-Revision-GPIO");
        		gpio_direction_input(board_config_pads[i]);
        	        pcb_rev |= (gpio_get_value(board_config_pads[i]) << i);
		}
		else
		{	
			gpio_request(board_config_pads[i], "SOM-Revision-GPIO");
                        gpio_direction_input(board_config_pads[i]);
                        bom_rev |= (gpio_get_value(board_config_pads[i]) << (i-4));
		}
	}

        printf ("\n");
        printf ("Board Info:\n");
        printf ("\tBSP Version     : %s\n", BSP_VERSION);
        printf ("\tSOM Version     : iW-PRFHZ-AP-01-R%x.%x\n",bom_rev+1,pcb_rev);
	printf ("\n");
}

int checkboard(void)
{
	puts("Board: iMX8QM IWG27M LPDDR4 ARM2\n");	
	
	print_bootinfo();

	/* Note:  After reloc, ipcHndl will no longer be valid.  If handle
	 *        returned by sc_ipc_open matches SC_IPC_CH, use this
	 *        macro (valid after reloc) for subsequent SCI calls.
	 */
	if (gd->arch.ipc_channel_handle != SC_IPC_CH) {
		printf("\nSCI error! Invalid handle\n");
	}

#ifdef SCI_FORCE_ABORT
	sc_rpc_msg_t abort_msg;

	puts("Send abort request\n");
	RPC_SIZE(&abort_msg) = 1;
	RPC_SVC(&abort_msg) = SC_RPC_SVC_ABORT;
	sc_ipc_write(SC_IPC_CH, &abort_msg);

	/* Close IPC channel */
	sc_ipc_close(SC_IPC_CH);
#endif /* SCI_FORCE_ABORT */

	return 0;
}

#ifdef CONFIG_FSL_HSIO

static void imx8qm_hsio_initialize(void)
{
	struct power_domain pd;
	int ret;

	if (!power_domain_lookup_name("hsio_sata0", &pd)) {
		ret = power_domain_on(&pd);
		if (ret)
			printf("hsio_sata0 Power up failed! (error = %d)\n", ret);
	}
}

#endif

void iwg27m_fdt_update(void *fdt)
{
	struct udevice *i2c_dev = NULL, *bus;
	uint8_t chip_gt1151 = 0x5D, chip_cst148 = 0x1A;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 2, &bus);
	if (ret)
		printf("\n Can't find I2C bus\n");

	/* IWG27M: Select 720p/1080p LCD based on Touch I2C slave address */
	if(!(dm_i2c_probe(bus, chip_gt1151, 0, &i2c_dev))) {
		fdt_del_node(fdt, fdt_path_offset(fdt, "/i2c@5a820000/cst148@1a"));
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi_bridge@56228000/panel@0"), "compatible", "raydium,rm68200");
	}

	else if(!(dm_i2c_probe(bus, chip_cst148, 0, &i2c_dev))) {
                fdt_del_node(fdt, fdt_path_offset(fdt, "/i2c@5a820000/goodix@5d"));
                fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi_bridge@56228000/panel@0"), "compatible", "raydium,rm67198");
        }

	/* IWG27M: Select LCD/HDMI based on boot argument */
	/*if (!strcmp("hdmi", getenv ("disp"))) {
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/dsi_phy@56228300"), "status", "disabled");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi@56228000"), "status", "disabled");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi_bridge@56228000"), "status", "disabled");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/hdmi@56268000"), "status", "okay");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/sound-hdmi"), "status", "okay");
	}
	else if (!strcmp("lcd", getenv ("disp"))) {
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/dsi_phy@56228300"), "status", "okay");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi@56228000"), "status", "okay");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/mipi_dsi_bridge@56228000"), "status", "okay");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/hdmi@56268000"), "status", "disabled");
		fdt_setprop_string(fdt, fdt_path_offset(fdt, "/sound-hdmi"), "status", "disabled");
	}*/
}

int board_init(void)
{
	/* IWG27M: MIPI-DSI: Correcting Reset Power Sequence for 1080p LCD */
	lcd_reset();
#ifdef CONFIG_FEC_MXC
	setup_fec(CONFIG_FEC_ENET_DEV);
#endif

#ifdef CONFIG_FSL_HSIO
	imx8qm_hsio_initialize();
#ifdef CONFIG_SCSI_AHCI_PLAT
	sata_init();
#endif
#endif
	return 0;
}

void board_quiesce_devices()
{
	const char *power_on_devices[] = {
		"dma_lpuart4",
	};

	power_off_pd_devices(power_on_devices, ARRAY_SIZE(power_on_devices));
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	puts("SCI reboot request");
	sc_pm_reboot(SC_IPC_CH, SC_PM_RESET_TYPE_COLD);
	while (1)
		putc('.');
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

int board_late_init(void)
{
	printboard_info();
	usb_otg_pwr_enable();
	usb_hub_pwr_enable();
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /*TODO*/
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/

/* Only Enable USB3 resources currently */
int board_usb_init(int index, enum usb_init_type init)
{
#ifndef CONFIG_DM_USB
	struct power_domain pd;
	int ret;

	/* Power on usb */
	if (!power_domain_lookup_name("conn_usb2", &pd)) {
		ret = power_domain_on(&pd);
		if (ret)
			printf("conn_usb2 Power up failed! (error = %d)\n", ret);
	}

	if (!power_domain_lookup_name("conn_usb2_phy", &pd)) {
		ret = power_domain_on(&pd);
		if (ret)
			printf("conn_usb2_phy Power up failed! (error = %d)\n", ret);
	}
#endif

	return 0;
}

#if defined(CONFIG_VIDEO_IMXDPUV1)
struct display_info_t const displays[] = {
};
size_t display_count = ARRAY_SIZE(displays);

#endif /* CONFIG_VIDEO_IMXDPUV1 */

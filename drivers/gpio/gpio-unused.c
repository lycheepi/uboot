/*
 * Copyright (c) 2018 iWave Systems Technologies Pvt. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * @file gpio-unused.c
 *
 * @brief Simple driver to set the unused GPIOs as input
 *
 * @ingroup GPIO
 */

#include <common.h>
#include <dm.h>
#include <dt-bindings/gpio/gpio.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <linux/bug.h>
#include <linux/ctype.h>

/* 
 * iw_gpio_probe - Probe method for the GPIO device.
 * @np: pointer to device tree node
 *
 * This function probes the Unused GPIOs in the device tree. It request GPIOs
 * as input. It returns 0, if all the GPIOs is requested as input
 * or a negative value if there is an error.
 */
static int iw_gpio_probe(struct udevice *dev)
{
	struct gpio_desc *num_gpios;
	int num_ctrl, ret;

	/* Fill GPIO pin array */
	num_ctrl = gpio_get_list_count(dev, "gpios");
	if (num_ctrl > 0) {         
		num_gpios = malloc(sizeof(struct gpio_desc) *num_ctrl);

		ret = gpio_request_list_by_name(dev, "gpios", num_gpios,
				num_ctrl, GPIOD_IS_IN);
		if (ret < 0) {
			pr_err("Can't get %s gpios! Error: %d", dev->name, ret);
			return ret;
		}
		free(num_gpios);            
	} else {
		dev_err(dev, "gpios DT property empty / missing\n");
		return -ENODEV;
	}	

	return 0;
}

static int iw_gpio_remove(struct udevice *dev)
{
	/* Platform not registerd return silently */
	return 0;
}

static const struct udevice_id iw_gpio_ids[] = {
	{ .compatible = "iwave,unused-gpios" },
	{ }
};

U_BOOT_DRIVER(iwgpio_unused) = {
	.name   = "iw_gpio",
	.id     = UCLASS_GPIO,
	.probe          = iw_gpio_probe,
	.of_match 	= iw_gpio_ids,
	.remove         = iw_gpio_remove,
};

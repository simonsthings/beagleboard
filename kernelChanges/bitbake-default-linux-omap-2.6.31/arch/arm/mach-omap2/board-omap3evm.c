/*
 * linux/arch/arm/mach-omap2/board-omap3evm.c
 *
 * Copyright (C) 2008 Texas Instruments
 *
 * Modified from mach-omap2/board-3430sdp.c
 *
 * Initial code: Syed Mohammed Khasim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/leds.h>

#include <linux/regulator/machine.h>
#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>
#include <linux/i2c/twl4030.h>
#include <linux/usb/otg.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/board.h>
#include <mach/mux.h>
#include <mach/usb.h>
#include <mach/common.h>
#include <mach/mcspi.h>
#include <mach/keypad.h>
#include <mach/display.h>

#include "sdram-micron-mt46h32m32lf-6.h"
#include "mmc-twl4030.h"

#define OMAP3_EVM_TS_GPIO	175

#define OMAP3EVM_ETHR_START	0x2c000000
#define OMAP3EVM_ETHR_SIZE	1024
#define OMAP3EVM_ETHR_GPIO_IRQ	176
#define OMAP3EVM_SMC911X_CS	5

static struct resource omap3evm_smc911x_resources[] = {
	[0] =	{
		.start	= OMAP3EVM_ETHR_START,
		.end	= (OMAP3EVM_ETHR_START + OMAP3EVM_ETHR_SIZE - 1),
		.flags	= IORESOURCE_MEM,
	},
	[1] =	{
		.start	= OMAP_GPIO_IRQ(OMAP3EVM_ETHR_GPIO_IRQ),
		.end	= OMAP_GPIO_IRQ(OMAP3EVM_ETHR_GPIO_IRQ),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device omap3evm_smc911x_device = {
	.name		= "smc911x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap3evm_smc911x_resources),
	.resource	= &omap3evm_smc911x_resources[0],
};

static inline void __init omap3evm_init_smc911x(void)
{
	int eth_cs;
	struct clk *l3ck;
	unsigned int rate;

	eth_cs = OMAP3EVM_SMC911X_CS;

	l3ck = clk_get(NULL, "l3_ck");
	if (IS_ERR(l3ck))
		rate = 100000000;
	else
		rate = clk_get_rate(l3ck);

	if (gpio_request(OMAP3EVM_ETHR_GPIO_IRQ, "SMC911x irq") < 0) {
		printk(KERN_ERR "Failed to request GPIO%d for smc911x IRQ\n",
			OMAP3EVM_ETHR_GPIO_IRQ);
		return;
	}

	gpio_direction_input(OMAP3EVM_ETHR_GPIO_IRQ);
}
/*
 * OMAP3EVM LCD Panel control signals
 */
#define OMAP3EVM_LCD_PANEL_LR		2
#define OMAP3EVM_LCD_PANEL_UD		3
#define OMAP3EVM_LCD_PANEL_INI		152
#define OMAP3EVM_LCD_PANEL_ENVDD	153
#define OMAP3EVM_LCD_PANEL_QVGA		154
#define OMAP3EVM_LCD_PANEL_RESB		155
#define OMAP3EVM_LCD_PANEL_BKLIGHT_GPIO	210
#define OMAP3EVM_DVI_PANEL_EN_GPIO	199

static int lcd_enabled;
static int dvi_enabled;

static void __init omap3_evm_display_init(void)
{
	int r;
	r = gpio_request(OMAP3EVM_LCD_PANEL_RESB, "lcd_panel_resb");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_resb\n");
		return;
	}
	gpio_direction_output(OMAP3EVM_LCD_PANEL_RESB, 1);

	r = gpio_request(OMAP3EVM_LCD_PANEL_INI, "lcd_panel_ini");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_ini\n");
		goto err_1;
	}
	gpio_direction_output(OMAP3EVM_LCD_PANEL_INI, 1);

	r = gpio_request(OMAP3EVM_LCD_PANEL_QVGA, "lcd_panel_qvga");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_qvga\n");
		goto err_2;
	}
	gpio_direction_output(OMAP3EVM_LCD_PANEL_QVGA, 0);

	r = gpio_request(OMAP3EVM_LCD_PANEL_LR, "lcd_panel_lr");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_lr\n");
		goto err_3;
	}
	gpio_direction_output(OMAP3EVM_LCD_PANEL_LR, 1);

	r = gpio_request(OMAP3EVM_LCD_PANEL_UD, "lcd_panel_ud");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_ud\n");
		goto err_4;
	}
	gpio_direction_output(OMAP3EVM_LCD_PANEL_UD, 1);

	r = gpio_request(OMAP3EVM_LCD_PANEL_ENVDD, "lcd_panel_envdd");
	if (r) {
		printk(KERN_ERR "failed to get lcd_panel_envdd\n");
		goto err_5;
	}

	return;

err_5:
	gpio_free(OMAP3EVM_LCD_PANEL_UD);
err_4:
	gpio_free(OMAP3EVM_LCD_PANEL_LR);
err_3:
	gpio_free(OMAP3EVM_LCD_PANEL_QVGA);
err_2:
	gpio_free(OMAP3EVM_LCD_PANEL_INI);
err_1:
	gpio_free(OMAP3EVM_LCD_PANEL_RESB);

}

static int omap3_evm_enable_lcd(struct omap_dss_device *dssdev)
{
	if (dvi_enabled) {
		printk(KERN_ERR "cannot enable LCD, DVI is enabled\n");
		return -EINVAL;
	}

	gpio_set_value(OMAP3EVM_LCD_PANEL_ENVDD, 0);
	gpio_set_value(OMAP3EVM_LCD_PANEL_BKLIGHT_GPIO, 1);
	lcd_enabled = 1;
	return 0;
}

static void omap3_evm_disable_lcd(struct omap_dss_device *dssdev)
{
	gpio_set_value(OMAP3EVM_LCD_PANEL_ENVDD, 1);
	gpio_set_value(OMAP3EVM_LCD_PANEL_BKLIGHT_GPIO, 0);
	lcd_enabled = 0;
}

static struct omap_dss_device omap3_evm_lcd_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "lcd",
	.driver_name		= "sharp_ls_panel",
	.phy.dpi.data_lines	= 18,
	.platform_enable	= omap3_evm_enable_lcd,
	.platform_disable	= omap3_evm_disable_lcd,
};

static int omap3_evm_enable_tv(struct omap_dss_device *dssdev)
{
	return 0;
}

static void omap3_evm_disable_tv(struct omap_dss_device *dssdev)
{
}

static struct omap_dss_device omap3_evm_tv_device = {
	.type			= OMAP_DISPLAY_TYPE_VENC,
	.name			= "tv",
	.driver_name		= "venc",
	.phy.venc.type		= OMAP_DSS_VENC_TYPE_SVIDEO,
	.platform_enable	= omap3_evm_enable_tv,
	.platform_disable	= omap3_evm_disable_tv,
};

static int omap3_evm_enable_dvi(struct omap_dss_device *dssdev)
{
	if (lcd_enabled) {
		printk(KERN_ERR "cannot enable DVI, LCD is enabled\n");
		return -EINVAL;
	}

	gpio_set_value(OMAP3EVM_DVI_PANEL_EN_GPIO, 1);
	dvi_enabled = 1;

	return 0;
}

static void omap3_evm_disable_dvi(struct omap_dss_device *dssdev)
{
	gpio_set_value(OMAP3EVM_DVI_PANEL_EN_GPIO, 0);
	dvi_enabled = 0;
}

static struct omap_dss_device omap3_evm_dvi_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "dvi",
	.driver_name		= "generic_panel",
	.phy.dpi.data_lines	= 24,
	.platform_enable	= omap3_evm_enable_dvi,
	.platform_disable	= omap3_evm_disable_dvi,
};

static struct omap_dss_device *omap3_evm_dss_devices[] = {
	&omap3_evm_lcd_device,
	&omap3_evm_tv_device,
	&omap3_evm_dvi_device,
};

static struct omap_dss_board_info omap3_evm_dss_data = {
	.num_devices	= ARRAY_SIZE(omap3_evm_dss_devices),
	.devices	= omap3_evm_dss_devices,
	.default_device	= &omap3_evm_lcd_device,
};

static struct platform_device omap3_evm_dss_device = {
	.name		= "omapdss",
	.id		= -1,
	.dev		= {
		.platform_data = &omap3_evm_dss_data,
	},
};

static struct twl4030_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.wires		= 4,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= 63,
	},
	{}	/* Terminator */
};

static struct gpio_led gpio_leds[] = {
	{
		.name			= "omap3evm::ledb",
		/* normally not visible (board underside) */
		.default_trigger	= "default-on",
		.gpio			= -EINVAL,	/* gets replaced */
		.active_low		= true,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds_gpio = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	},
};


static int omap3evm_twl_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	omap_cfg_reg(L8_34XX_GPIO63);
	mmc[0].gpio_cd = gpio + 0;
	twl4030_mmc_init(mmc);

	/*
	 * Most GPIOs are for USB OTG.  Some are mostly sent to
	 * the P2 connector; notably LEDA for the LCD backlight.
	 */

	/* TWL4030_GPIO_MAX + 0 == ledA, LCD Backlight control */
	gpio_request(gpio + TWL4030_GPIO_MAX, "EN_LCD_BKL");
	gpio_direction_output(gpio + TWL4030_GPIO_MAX, 0);

	/* gpio + 7 == DVI Enable */
	gpio_request(gpio + 7, "EN_DVI");
	gpio_direction_output(gpio + 7, 0);

	/* TWL4030_GPIO_MAX + 1 == ledB (out, active low LED) */
	gpio_leds[2].gpio = gpio + TWL4030_GPIO_MAX + 1;

	platform_device_register(&leds_gpio);

	return 0;
}

static struct twl4030_gpio_platform_data omap3evm_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.use_leds	= true,
	.setup		= omap3evm_twl_gpio_setup,
};

static struct twl4030_usb_data omap3evm_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};

static int omap3evm_keymap[] = {
	KEY(0, 0, KEY_LEFT),
	KEY(0, 1, KEY_RIGHT),
	KEY(0, 2, KEY_A),
	KEY(0, 3, KEY_B),
	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_UP),
	KEY(1, 2, KEY_E),
	KEY(1, 3, KEY_F),
	KEY(2, 0, KEY_ENTER),
	KEY(2, 1, KEY_I),
	KEY(2, 2, KEY_J),
	KEY(2, 3, KEY_K),
	KEY(3, 0, KEY_M),
	KEY(3, 1, KEY_N),
	KEY(3, 2, KEY_O),
	KEY(3, 3, KEY_P)
};

static struct twl4030_keypad_data omap3evm_kp_data = {
	.rows		= 4,
	.cols		= 4,
	.keymap		= omap3evm_keymap,
	.keymapsize	= ARRAY_SIZE(omap3evm_keymap),
	.rep		= 1,
};

static struct twl4030_madc_platform_data omap3evm_madc_data = {
	.irq_line	= 1,
};

static struct regulator_consumer_supply omap3_evm_vdda_dac_supply = {
	.supply		= "vdda_dac",
	.dev		= &omap3_evm_dss_device.dev,
};

/* VDAC for DSS driving S-Video */
static struct regulator_init_data omap3_evm_vdac = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &omap3_evm_vdda_dac_supply,
};

/* VPLL2 for digital video outputs */
static struct regulator_consumer_supply omap3_evm_vpll2_supply = {
	.supply		= "vdvi",
	.dev		= &omap3_evm_lcd_device.dev,
};

static struct regulator_init_data omap3_evm_vpll2 = {
	.constraints = {
		.name			= "VDVI",
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &omap3_evm_vpll2_supply,
};

static struct twl4030_platform_data omap3evm_twldata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.keypad		= &omap3evm_kp_data,
	.madc		= &omap3evm_madc_data,
	.usb		= &omap3evm_usb_data,
	.gpio		= &omap3evm_gpio_data,
	.vdac		= &omap3_evm_vdac,
	.vpll2		= &omap3_evm_vpll2,
};

static struct i2c_board_info __initdata omap3evm_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl4030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &omap3evm_twldata,
	},
};

static int __init omap3_evm_i2c_init(void)
{
	omap_register_i2c_bus(1, 2600, omap3evm_i2c_boardinfo,
			ARRAY_SIZE(omap3evm_i2c_boardinfo));
	omap_register_i2c_bus(2, 400, NULL, 0);
	omap_register_i2c_bus(3, 400, NULL, 0);
	return 0;
}

static void ads7846_dev_init(void)
{
	if (gpio_request(OMAP3_EVM_TS_GPIO, "ADS7846 pendown") < 0)
		printk(KERN_ERR "can't get ads7846 pen down GPIO\n");

	gpio_direction_input(OMAP3_EVM_TS_GPIO);

	omap_set_gpio_debounce(OMAP3_EVM_TS_GPIO, 1);
	omap_set_gpio_debounce_time(OMAP3_EVM_TS_GPIO, 0xa);
}

static int ads7846_get_pendown_state(void)
{
	return !gpio_get_value(OMAP3_EVM_TS_GPIO);
}

struct ads7846_platform_data ads7846_config = {
	.x_max			= 0x0fff,
	.y_max			= 0x0fff,
	.x_plate_ohms		= 180,
	.pressure_max		= 255,
	.debounce_max		= 10,
	.debounce_tol		= 3,
	.debounce_rep		= 1,
	.get_pendown_state	= ads7846_get_pendown_state,
	.keep_vref_on		= 1,
	.settle_delay_usecs	= 150,
};

static struct omap2_mcspi_device_config ads7846_mcspi_config = {
	.turbo_mode	= 0,
	.single_channel	= 1,	/* 0: slave, 1: master */
};

struct spi_board_info omap3evm_spi_board_info[] = {
	[0] = {
		.modalias		= "ads7846",
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 1500000,
		.controller_data	= &ads7846_mcspi_config,
		.irq			= OMAP_GPIO_IRQ(OMAP3_EVM_TS_GPIO),
		.platform_data		= &ads7846_config,
	},
};

static struct omap_board_config_kernel omap3_evm_config[] __initdata = {
};

static void __init omap3_evm_init_irq(void)
{
	omap_board_config = omap3_evm_config;
	omap_board_config_size = ARRAY_SIZE(omap3_evm_config);
	omap2_init_common_hw(mt46h32m32lf6_sdrc_params, NULL);
	omap_init_irq();
	omap_gpio_init();
	omap3evm_init_smc911x();
}

static struct platform_device *omap3_evm_devices[] __initdata = {
	&omap3_evm_dss_device,
	&omap3evm_smc911x_device,
};

static struct ehci_hcd_omap_platform_data ehci_pdata __initconst = {

	.port_mode[0] = EHCI_HCD_OMAP_MODE_UNKNOWN,
	.port_mode[1] = EHCI_HCD_OMAP_MODE_PHY,
	.port_mode[2] = EHCI_HCD_OMAP_MODE_UNKNOWN,

	.chargepump = false,
	.phy_reset  = true,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = 135,
	.reset_gpio_port[2]  = -EINVAL
};

static void __init omap3_evm_init(void)
{
	omap3_evm_i2c_init();

	platform_add_devices(omap3_evm_devices, ARRAY_SIZE(omap3_evm_devices));

	spi_register_board_info(omap3evm_spi_board_info,
				ARRAY_SIZE(omap3evm_spi_board_info));

	omap_serial_init();
#ifdef CONFIG_NOP_USB_XCEIV
	/* OMAP3EVM uses ISP1504 phy and so register nop transceiver */
	usb_nop_xceiv_register();
#endif
	usb_musb_init();
	/* Setup EHCI phy reset padconfig */
	omap_cfg_reg(AF4_34XX_GPIO135_OUT);
	usb_ehci_init(&ehci_pdata);
	ads7846_dev_init();
	omap3_evm_display_init();
}

static void __init omap3_evm_map_io(void)
{
	omap2_set_globals_343x();
	omap2_map_common_io();
}

MACHINE_START(OMAP3EVM, "OMAP3 EVM")
	/* Maintainer: Syed Mohammed Khasim - Texas Instruments */
	.phys_io	= 0x48000000,
	.io_pg_offst	= ((0xd8000000) >> 18) & 0xfffc,
	.boot_params	= 0x80000100,
	.map_io		= omap3_evm_map_io,
	.init_irq	= omap3_evm_init_irq,
	.init_machine	= omap3_evm_init,
	.timer		= &omap_timer,
MACHINE_END

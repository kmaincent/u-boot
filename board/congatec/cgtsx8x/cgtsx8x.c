// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <linux/libfdt.h>
#include <fsl_esdhc_imx.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/snvs_security_sc.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <usb.h>
#include <netdev.h>
#include <power-domain.h>

DECLARE_GLOBAL_DATA_PTR;

#define ENET_INPUT_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart2_pads[] = {
	SC_P_UART2_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART2_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

struct gpio_exp_pin
{
	struct gpio_desc desc;
	char *dt_lookup_name;
	char *name;
	ulong dir_flags;
	uint8_t skip_setup;
};

static struct gpio_exp_pin gpio_exp0_pin_list[] = {
	{
		.name = "gpio_exp_gbe0_rst_n",
		.dt_lookup_name = "gpio@73_0",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_gbe1_rst_n",
		.dt_lookup_name = "gpio@73_1",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_usb_grst_n",
		.dt_lookup_name = "gpio@73_2",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_espi_alert0_n",
		.dt_lookup_name = "gpio@73_3",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_espi_alert1_n",
		.dt_lookup_name = "gpio@73_4",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_bt_rf_disable_n",
		.dt_lookup_name = "gpio@73_5",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_wifi_rf_disable_n",
		.dt_lookup_name = "gpio@73_6",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_wifi_pwrdwn_n",
		.dt_lookup_name = "gpio@73_7",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
};

static struct gpio_exp_pin gpio_exp1_pin_list[] = {
	{
		.name = "gpio_exp_lcd0_gpio",
		.dt_lookup_name = "gpio@72_0",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_lcd1_gpio",
		.dt_lookup_name = "gpio@72_1",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_i2s2_rst_n",
		.dt_lookup_name = "gpio@72_2",
		.dir_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_espi_reset_n",
		.dt_lookup_name = "gpio@72_3",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_charging_n",
		.dt_lookup_name = "gpio@72_4",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_charger_prsnt_n",
		.dt_lookup_name = "gpio@72_5",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_batlow_n",
		.dt_lookup_name = "gpio@72_6",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
	{
		.name = "gpio_exp_sleep_n",
		.dt_lookup_name = "gpio@72_7",
		.dir_flags = GPIOD_IS_IN,
		.skip_setup = 0
	},
};

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate = SC_80MHZ;
	int ret;

	/* Set UART0 clock root to 80 MHz */
	ret = sc_pm_setup_uart(SC_R_UART_2, rate);
	if (ret)
		return ret;

	setup_iomux_uart();

/* Dual bootloader feature will require CAAM access, but JR0 and JR1 will be
 * assigned to seco for imx8, use JR3 instead.
 */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_DUAL_BOOTLOADER)
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3_OUT, SC_PM_PW_MODE_ON);
#endif

	return 0;
}

#if CONFIG_IS_ENABLED(DM_GPIO)
static iomux_cfg_t smarc_edge_gpios[] = {
    /* SMARC EDGE P115 GPIO7, Ball P32 LSIO.GPIO1.IO01 */
    SC_P_SPI2_SDO | MUX_MODE_ALT(4) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

/* GPIOs @ SMARC edge connector */
#define GPIO_SMARC_EDGE_P115_GPIO7 IMX_GPIO_NR(1, 1)

#define str(s) #s
static void board_gpio_init(void)
{
	int ret;

	/* GPIOs @ SMARC edge connector */
	imx8_iomux_setup_multiple_pads( smarc_edge_gpios,
					ARRAY_SIZE(smarc_edge_gpios) );

	ret = gpio_request( GPIO_SMARC_EDGE_P115_GPIO7,
			    "sedge_P115_GPIO7" );
	if (ret)
		printf( "ERROR: requesting GPIO %s failed \n",
			str(GPIO_SMARC_EDGE_P115_GPIO7) );

	gpio_direction_input( GPIO_SMARC_EDGE_P115_GPIO7 );
}
#else
static inline void board_gpio_init(void) {}
#endif

#ifdef CONFIG_DM_PCA953X

static void board_gpio_exp_init( struct gpio_exp_pin *gpio_exp0_pin_list,
				 uint8_t pin_list_size )
{
	int ret;
	int lc = 0;

	for( lc = 0; lc < pin_list_size; lc++ )
	{
		struct gpio_desc *desc = &gpio_exp0_pin_list[lc].desc;

		if( gpio_exp0_pin_list[lc].skip_setup )
			continue;

#ifdef DEBUG
		printf( "DM_GPIO SETUP: %s\n",
			gpio_exp0_pin_list[lc].dt_lookup_name );
#endif

		ret = dm_gpio_lookup_name( gpio_exp0_pin_list[lc].dt_lookup_name,
					   desc );
		if (ret)
		{
			printf( "ERR: DM_GPIO LOOKUP FAILED: %s\n",
				gpio_exp0_pin_list[lc].dt_lookup_name );
			continue;
		}

		ret = dm_gpio_request( desc,
				       gpio_exp0_pin_list[lc].name );
		if (ret)
			printf( "ERR: DM_GPIO REQ FAILED: %s\n",
				gpio_exp0_pin_list[lc].dt_lookup_name );

		ret = dm_gpio_set_dir_flags( desc,
					     gpio_exp0_pin_list[lc].dir_flags );
		if (ret)
			printf( "ERR: DM_GPIO SD FAILED: %s\n",
				gpio_exp0_pin_list[lc].dt_lookup_name );
	}
}

static void gpio_exp_trigger_reset_lines(void)
{
    struct gpio_desc *pdesc = NULL;

    /* gpio_exp_usb_grst_n, 55ms reset pulse (low) */
    pdesc = &gpio_exp0_pin_list[2].desc;
    dm_gpio_set_value( pdesc, 0 );
    mdelay( 50 );
    dm_gpio_set_value( pdesc, 1 );

    /* gpio_exp_wifi_pwrdwn_n, 55ms reset pulse (low) */
    pdesc = &gpio_exp0_pin_list[7].desc;
    dm_gpio_set_value( pdesc, 0 );
    mdelay( 50 );
    dm_gpio_set_value( pdesc, 1 );
}

#endif /* CONFIG_DM_PCA953X */

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>

#ifndef CONFIG_DM_ETH
static iomux_cfg_t pad_enet1[] = {
	SC_P_SPDIF0_TX | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_SPDIF0_RX | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX3_RX2 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX2_RX3 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX1 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX0 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_SCKR | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_TX4_RX1 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_TX5_RX0 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_FST  | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_SCKT | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_FSR  | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),

	/* 25MHz refclk */
	SC_P_SPDIF0_EXT_CLK | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),

	/* Shared MDIO */
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
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

	/* 25MHz refclk */
	SC_P_ENET0_REFCLK_125M_25M | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),

	/* Shared MDIO */
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
};

static void setup_iomux_fec0(void)
{
	imx8_iomux_setup_multiple_pads( pad_enet0,
					ARRAY_SIZE(pad_enet0) );
}

static void setup_iomux_fec1(void)
{
	imx8_iomux_setup_multiple_pads( pad_enet1,
					ARRAY_SIZE(pad_enet1) );
}

static void enet_device_phy_reset( struct gpio_desc *pdesc )
{
	/* requested once @ board_init / board_gpio_exp_init */

        dm_gpio_set_value( pdesc, 0 );
	udelay(50);
        dm_gpio_set_value( pdesc, 1 );

	/* The board has a long delay for this reset to become stable */
	mdelay(200);
}

typedef void (*cb_iomux_fec)( void );
static int board_eth_setup( const char *fec_pd_name,
			    bd_t *bis,
			    int fec_enet_dev,
			    int fec_mxc_phyaddr,
			    uint32_t fec_base,
			    cb_iomux_fec setup_iomux_fec )
{
	int ret;
	struct power_domain pd;

	printf("[%s] %d\n", __func__, __LINE__);

	if ( !power_domain_lookup_name(fec_pd_name, &pd) )
	{
		ret = power_domain_on(&pd);
		if ( ret != 0 )
		{
			printf( "%s Power up failed! (error = %d)\n",
				fec_pd_name,
				ret );
			return ret;
		}
	}

	setup_iomux_fec();

	ret = fecmxc_initialize_multi( bis,
				       fec_enet_dev,
				       fec_mxc_phyaddr,
				       fec_base );
	if (ret)
		printf("FEC%i MXC: %s: failed\n", fec_enet_dev ,__func__);

	return ret;
}

static int setup_fec(int ind)
{
	/* Reset ENET PHYs */

#ifdef CONFIG_DM_PCA953X
#ifdef CONFIG_CGT_ENABLE_FEC0
	enet_device_phy_reset( &gpio_exp0_pin_list[0].desc );
#endif /* CONFIG_CGT_ENABLE_FEC0 */

#ifdef CONFIG_CGT_ENABLE_FEC1
	enet_device_phy_reset( &gpio_exp0_pin_list[1].desc );
#endif /* CONFIG_CGT_ENABLE_FEC1 */
#endif /* CONFIG_DM_PCA953X */

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = 1;

	printf("[%s] %d\n", __func__, __LINE__);

#ifdef CONFIG_CGT_ENABLE_FEC0
	ret = board_eth_setup( IMX_FEC0_PD_NAME,
			       bis,
			       IMX_FEC0_ENET_DEV,
			       IMX_FEC0_MXC_PHYADDR,
			       IMX_FEC0_BASE,
			       setup_iomux_fec0 );
	if ( ret != 0 )
		return ret;
#endif /* CONFIG_CGT_ENABLE_FEC0 */

#ifdef CONFIG_CGT_ENABLE_FEC1
	ret = board_eth_setup( IMX_FEC1_PD_NAME,
			       bis,
			       IMX_FEC1_ENET_DEV,
			       IMX_FEC1_MXC_PHYADDR,
			       IMX_FEC1_BASE,
			       setup_iomux_fec1 );

	if ( ret != 0 )
		return ret;
#endif /* CONFIG_CGT_ENABLE_FEC1 */

	return ret;
}
#endif

int board_phy_config(struct phy_device *phydev)
{
	u16 val = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x8;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

int checkboard(void)
{
#ifdef CONFIG_TARGET_CONGA_DUAL
	puts("Board: conga-SMX8-X aka SX8X Dual\n");
#else
	puts("Board: conga-SMX8-X aka SX8X Quad\n");
#endif

	build_info();
	print_bootinfo();

	return 0;
}

#ifdef CONFIG_USB

#ifdef CONFIG_USB_TCPC
struct gpio_desc type_sel_desc;
static iomux_cfg_t ss_mux_gpio[] = {
	SC_P_ENET0_REFCLK_125M_25M | MUX_MODE_ALT(4) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

struct tcpc_port port;
struct tcpc_port_config port_config = {
	.i2c_bus = 1,
	.addr = 0x50,
	.port_type = TYPEC_PORT_DFP,
};

void ss_mux_select(enum typec_cc_polarity pol)
{
	if (pol == TYPEC_POLARITY_CC1)
		dm_gpio_set_value(&type_sel_desc, 0);
	else
		dm_gpio_set_value(&type_sel_desc, 1);
}

static void setup_typec(void)
{
	int ret;
	struct gpio_desc typec_en_desc;

	imx8_iomux_setup_multiple_pads(ss_mux_gpio, ARRAY_SIZE(ss_mux_gpio));
	ret = dm_gpio_lookup_name("GPIO5_9", &type_sel_desc);
	if (ret) {
		printf("%s lookup GPIO5_9 failed ret = %d\n", __func__, ret);
		return;
	}

	ret = dm_gpio_request(&type_sel_desc, "typec_sel");
	if (ret) {
		printf("%s request typec_sel failed ret = %d\n", __func__, ret);
		return;
	}

	dm_gpio_set_dir_flags(&type_sel_desc, GPIOD_IS_OUT);

	ret = dm_gpio_lookup_name("gpio@1a_7", &typec_en_desc);
	if (ret) {
		printf("%s lookup gpio@1a_7 failed ret = %d\n", __func__, ret);
		return;
	}

	ret = dm_gpio_request(&typec_en_desc, "typec_en");
	if (ret) {
		printf("%s request typec_en failed ret = %d\n", __func__, ret);
		return;
	}

	/* Enable SS MUX */
	dm_gpio_set_dir_flags(&typec_en_desc, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	tcpc_init(&port, port_config, &ss_mux_select);
}
#endif

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	if (index == 1) {
		if (init == USB_INIT_HOST) {
#ifdef CONFIG_USB_TCPC
			ret = tcpc_setup_dfp_mode(&port);
#endif
#ifdef CONFIG_USB_CDNS3_GADGET
		} else {
#ifdef CONFIG_USB_TCPC
			ret = tcpc_setup_ufp_mode(&port);
			printf("%d setufp mode %d\n", index, ret);
#endif
#endif
		}
	}

	return ret;

}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;

	if (index == 1) {
		if (init == USB_INIT_HOST) {
#ifdef CONFIG_USB_TCPC
			ret = tcpc_disable_src_vbus(&port);
#endif
		}
	}

	return ret;
}
#endif

int board_init(void)
{
	board_gpio_init();

#ifdef CONFIG_DM_PCA953X
	board_gpio_exp_init( (struct gpio_exp_pin *)&gpio_exp0_pin_list,
			     sizeof(gpio_exp0_pin_list) / sizeof(struct gpio_exp_pin) );

	board_gpio_exp_init( (struct gpio_exp_pin *)&gpio_exp1_pin_list,
			     sizeof(gpio_exp1_pin_list) / sizeof(struct gpio_exp_pin) );
#endif /* CONFIG_DM_PCA953X */

#if defined(CONFIG_USB) && defined(CONFIG_USB_TCPC)
	setup_typec();
#endif

#ifdef CONFIG_SNVS_SEC_SC_AUTO
	{
		int ret = snvs_security_sc_init();

		if (ret)
			return ret;
	}
#endif

#if IS_ENABLED(CONFIG_FEC_MXC)
	setup_fec(0xFF);
#endif

	return 0;
}

void board_quiesce_devices(void)
{
	const char *power_on_devices[] = {
		"dma_lpuart2",

		/* HIFI DSP boot */
		"audio_sai0",
		"audio_ocram",
	};

	power_off_pd_devices(power_on_devices, ARRAY_SIZE(power_on_devices));
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	sc_pm_reboot(-1, SC_PM_RESET_TYPE_COLD);
	while(1);

}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "SX8X");
#ifdef CONFIG_TARGET_CONGA_DUAL
	env_set("board_rev", "iMX8DX");
#else
	env_set("board_rev", "iMX8QXP");
#endif
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_CGT_CM4_PART_DET
	/* autodetection of cm4 partition status */
	bool m4_boot = false;
	m4_boot = check_m4_parts_boot();
	printf( "CM4 partition state: %s\n",
		m4_boot ? "started" : "not started" );
#endif /* CONFIG_CGT_CM4_PART_DET */

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

#ifdef CONFIG_DM_PCA953X
        /* create reset pulses (gpio expanders) */
        gpio_exp_trigger_reset_lines();
#endif /* CONFIG_DM_PCA953X */

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

#ifdef CONFIG_ANDROID_SUPPORT
bool is_power_key_pressed(void) {
	sc_bool_t status = SC_FALSE;

	sc_misc_get_button_status(-1, &status);
	return (bool)status;
}
#endif

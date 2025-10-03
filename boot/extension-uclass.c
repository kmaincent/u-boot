// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Köry Maincent <kory.maincent@bootlin.com>
 */

#include <alist.h>
#include <command.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass.h>
#include <env.h>
#include <extension_board.h>
#include <fdt_support.h>
#include <malloc.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

/* For now, bind the extension device manually if none are found */
static struct udevice *extension_get_dev(void)
{
	struct driver *drv = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct udevice *dev;
	int i, ret;

	/* These are not needed before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return NULL;

	uclass_first_device(UCLASS_EXTENSION, &dev);
	if (dev)
		return dev;

	/* Create the extension device if not already bound */
	for (i = 0; i < n_ents; i++, drv++) {
		if (drv->id == UCLASS_EXTENSION) {
			char name[32];

			snprintf(name, sizeof(name), "%s_dev", drv->name);
			ret = device_bind_driver(gd->dm_root, drv->name,
						 name, &dev);
			if (ret) {
				printf("Bind extension driver %s error=%d\n",
				       drv->name, ret);
				return NULL;
			}

			ret = device_probe(dev);
			if (ret) {
				printf("Probe extension driver %s error=%d\n",
				       drv->name, ret);
				return NULL;
			}

			/* We manage only one extension driver for now */
			return dev;
		}
	}

	return NULL;
}

struct alist *extension_get_list(void)
{
	struct udevice *dev = extension_get_dev();

	if (!dev)
		return NULL;

	return dev_get_priv(dev);
}

int extension_probe(struct udevice *dev)
{
	struct alist *extension_list = dev_get_priv(dev);

	alist_init_struct(extension_list, struct extension);
	return 0;
}

int extension_remove(struct udevice *dev)
{
	struct alist *extension_list = dev_get_priv(dev);

	alist_uninit(extension_list);
	return 0;
}

int extension_scan(void)
{
	struct alist *extension_list = extension_get_list();
	struct udevice *dev = extension_get_dev();
	const struct extension_ops *ops;

	if (!dev || !extension_list)
		return -ENODEV;

	ops = device_get_ops(dev);
	if (!ops->scan)
		return -ENODEV;

	alist_empty(extension_list);
	return ops->scan(extension_list);
}

int extension_apply(struct fdt_header *working_fdt, ulong size)
{
	struct fdt_header *blob;
	ulong overlay_addr;
	int ret;

	overlay_addr = env_get_hex("extension_overlay_addr", 0);
	if (!overlay_addr) {
		printf("Environment extension_overlay_addr is missing\n");
		return -EINVAL;
	}

	fdt_shrink_to_minimum(working_fdt, size);

	blob = map_sysmem(overlay_addr, 0);
	if (!fdt_valid(&blob)) {
		printf("Invalid overlay devicetree\n");
		return -EINVAL;
	}

	/* Apply method prints messages on error */
	ret = fdt_overlay_apply_verbose(working_fdt, blob);
	if (ret)
		printf("Failed to apply overlay\n");

	return ret;
}

UCLASS_DRIVER(extension) = {
	.name	= "extension",
	.id	= UCLASS_EXTENSION,
};

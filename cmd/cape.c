// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, kory.maincent@bootlin.com
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <cape_support.h>

static LIST_HEAD(cape_list);

static int do_cape_list(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	int i;
	struct cape *cape;

	if (list_empty(&cape_list)) {
		printf("No cape registered - Please run \"cape scan\"\n");
		return CMD_RET_SUCCESS;
	}

	list_for_each_entry(cape, &cape_list, list) {
		printf("Cape %d: %s\n", i++, cape->name);
		printf("\tManufacturer: \t\t%s\n", cape->owner);
		printf("\tVersion: \t\t%s\n", cape->version);
		printf("\tDevicetree overlay: \t%s\n", cape->overlay);
		printf("\tOther information: \t%s\n", cape->other);
	}
	return CMD_RET_SUCCESS;
}

static int do_cape_scan(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	struct cape *cape, *next;
	list_for_each_entry_safe(cape, next, &cape_list, list) {
		list_del(&cape->list);
		free(cape);
	}
	int cape_num = cape_board_scan(&cape_list);
	printf("Found %d cape.\n", cape_num);

	return CMD_RET_SUCCESS;
}

static int do_cape_apply(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
#ifndef CONFIG_CMD_FDT
	printf("fdt command no enable, can not apply cape overlay\n");
	return CMD_RET_FAILURE;
#else

	if (argc < 2)
		return CMD_RET_USAGE;

	struct cape *cape;
	char cmd[32], *overlay_cmd;
	int i = 0, num_cape;

	overlay_cmd = env_get("cape_overlay_cmd");
	if (!overlay_cmd) {
		printf("Environment cape_overlay_cmd is missing\n");
		return CMD_RET_FAILURE;
	}

	if (!env_get("cape_overlay_addr")) {
		printf("Environment cape_overlay_addr is missing\n");
		return CMD_RET_FAILURE;
	}

	snprintf(cmd, 32, "fdt resize 0x20000");
	if (run_command(cmd, 0) != 0)
		return CMD_RET_FAILURE;

	if (strcmp(argv[1], "all") == 0) {
		list_for_each_entry(cape, &cape_list, list) {
			env_set("cape_overlay_name", cape->overlay);
			if (run_command(overlay_cmd, 0) != 0)
				return CMD_RET_FAILURE;

			sprintf(cmd, "fdt apply $cape_overlay_addr");
			if (run_command(cmd, 0) != 0)
				return CMD_RET_FAILURE;
		}
	}
	else {
		num_cape = simple_strtol(argv[1], NULL, 10);
		list_for_each_entry(cape, &cape_list, list) {
			if (i++ == num_cape)
				break;
		}

		if (num_cape >= i) {
			printf("Wrong number of cape\n");
			return CMD_RET_FAILURE;
		}

		env_set("cape_overlay_name", cape->overlay);
		if (run_command(overlay_cmd, 0) != 0)
			return CMD_RET_FAILURE;

		sprintf(cmd, "fdt apply ${cape_overlay_addr}");
		if (run_command(cmd, 0) != 0)
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
#endif /* CONFIG_CMD_FDT */
}

static struct cmd_tbl cmd_cape[] = {
	U_BOOT_CMD_MKENT(scan, 1, 1, do_cape_scan, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 0, do_cape_list, "", ""),
	U_BOOT_CMD_MKENT(apply, 2, 0, do_cape_apply, "", ""),
};

static int do_capeops(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	struct cmd_tbl *cp;

	/* Drop the cape command */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_cape, ARRAY_SIZE(cmd_cape));

        if (cp)
            return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	cape, 3, 1, do_capeops,
	"Cape management sub system",
	"scan\n"
	"cape list - lists available capes\n"
	"cape apply <cape number|all> - apply the selected cape or all capes\n"
	"cape get_overlay <cape number|all> - apply the selected cape or all capes\n"
);

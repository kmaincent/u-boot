// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, kory.maincent@bootlin.com
 */


#ifndef __CAPE_SUPPORT_H
#define __CAPE_SUPPORT_H

#define TEXT_SIZE 32

struct cape {
	struct list_head list;
	char name[TEXT_SIZE];
	char owner[TEXT_SIZE];
	char version[TEXT_SIZE];
	char overlay[TEXT_SIZE];
	char other[TEXT_SIZE];
};

/**
 * Add system-specific function to scan cape.
 *
 * This function is called if CONFIG_CMD_CAPE is defined
 *
 * @param cape_list	List of cape to update.
 * @return the number of cape.
 */
int cape_board_scan(struct list_head *cape_list);

#endif /* __CAPE_SUPPORT_H */

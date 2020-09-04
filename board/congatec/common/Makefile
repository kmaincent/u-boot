#
# Copyright 2017 NXP
#
#
# SPDX-License-Identifier:	GPL-2.0+

MINIMAL=

ifdef CONFIG_SPL_BUILD

ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif

endif

ifdef MINIMAL

# necessary to create built-in.o
obj- := __dummy__.o

else

obj-y += ../../freescale/common/mmc.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TCPC) += ../../freescale/common/tcpc.o
endif

endif

Q=@

include ../makefile.config
include ../mk/defconfig.mk

INCLUDES += -I$(CDIR)/../
INCLUDES += -I$(CDIR)/../core
INCLUDES += -I$(CDIR)/

MODULE=linux

include ../mk/colors.mk
include ../mk/functions.mk
include ../mk/module.mk

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.lib
	$(Q)rm -f *.map


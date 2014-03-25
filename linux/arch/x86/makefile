Q=@

include ../../../makefile.config
include ../../../mk/defconfig.mk

INCLUDES += -I$(CDIR)/../../../
INCLUDES += -I$(CDIR)/../../../arch/x86
INCLUDES += -I$(CDIR)/

MODULE=jump_to_kernel

include ../../../mk/colors.mk
include ../../../mk/functions.mk
include ../../../mk/module.mk

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.lib
	$(Q)rm -f *.map


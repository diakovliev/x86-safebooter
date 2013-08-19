Q=@

include ../makefile.config
include ../mk/defconfig.mk
include ../mk/functions.mk

INCLUDES += -I$(CDIR)/../
INCLUDES += -I$(CDIR)/../core
INCLUDES += -I$(CDIR)/../linux
INCLUDES += -I$(CDIR)/

MODULE=main

include ../mk/module.mk

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.lib


Q=@

include ../makefile.config
include ../mk/defconfig.mk

INCLUDES += -I$(CDIR)/../
INCLUDES += -I$(CDIR)/../core
INCLUDES += -I$(CDIR)/../linux
INCLUDES += -I$(CDIR)/

MODULE=main

include ../mk/functions.mk
include ../mk/module.mk

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.lib


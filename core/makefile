Q=@

include ../makefile.config
include ../mk/defconfig.mk
include ../mk/functions.mk

INCLUDES += -I$(CDIR)/../
INCLUDES += -I$(CDIR)/

MODULE=core.lib

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(SRCS))))

build: $(MODULE)

$(MODULE): $(OBJS)
	$(Q)$(call LD_LIB_CMD, $(OBJS))

$(OBJS): $(SRCS)
	$(Q)$(foreach file,$(SRCS),$(call GCC_CMD,$(file));)

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.lib


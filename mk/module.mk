HDRS=$(wildcard *.h)
SRCS=$(wildcard *.c)
SRCS+=$(wildcard *.S)
OBJS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(SRCS))))

$(MODULE): $(OBJS)
	$(Q)$(call LD_LIB_CMD, $(OBJS))

$(OBJS): $(SRCS) $(HDRS)
	$(Q)$(foreach file,$(SRCS),$(call GCC_CMD,$(file)) && ) true


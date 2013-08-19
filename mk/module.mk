build: pre-build $(MODULE) post-build

HDRS=$(wildcard *.h)
SRCS=$(wildcard *.c)
SRCS+=$(wildcard *.S)
OBJS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(SRCS))))

pre-build:
	$(Q)$(GREEN) && printf "** %s **\n" $(MODULE) && $(NORMAL)

post-build:
	$(Q)$(GREEN) && printf "** %s IS READY **\n" $(MODULE) && $(NORMAL)

$(MODULE): $(OBJS)
	$(Q)$(call LD_LIB_CMD, $(OBJS))

$(OBJS): $(SRCS) $(HDRS)
	$(Q)$(foreach file,$(SRCS),$(call GCC_CMD,$(file)) && ) true


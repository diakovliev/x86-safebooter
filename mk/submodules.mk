SUBDIRS		= $(shell find -type d | tail -n+2 | sort)
SUBMODULES	= $(shell for i in $(call SUBDIRS); do [ -f $$i/makefile ] && echo $$i; done)
TOLINK		= $(shell for i in $(call SUBDIRS); do [ -f $$i/*.lib ] && echo $$i/*.lib; done)

whole_submodules_build:
	$(Q)$(foreach directory, $(call SUBMODULES), make -C $(directory) -f makefile build && )$(GREEN) && printf "** ALL SUBMODULES READY **\n" && $(NORMAL)

whole_submodules_clean:
	$(Q)$(foreach directory, $(SUBMODULES), make -C $(directory) -f makefile clean ; )


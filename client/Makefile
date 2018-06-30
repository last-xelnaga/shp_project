
SUBDIRS := $(wildcard */.)

COMMANDS := all clean

$(COMMANDS): $(SUBDIRS)

$(SUBDIRS):
	make -C $@ $(MAKECMDGOALS)

.PHONY: $(COMMANDS) $(SUBDIRS)

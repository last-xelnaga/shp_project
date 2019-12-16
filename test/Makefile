
TARGET = shp_test
OBJDIR = $(shell pwd)/obj

INC    = -I ../external/googletest/googletest/include
LIB    = -L ../external/googletest/bazel-bin -l:libgtest.a -l:libgtest_main.a -lpthread
FLAGS  = -DLINUX_TARGET


SRCCPP = \
    file_utils_test.cpp \

COMMON = ../common

# sources
#include $(COMMON)/crypto/.crypto_rules
#include $(COMMON)/network/.server_rules
include $(COMMON)/utils/.utils_rules

# global rules
include $(COMMON)/build/.global_rules

# dependencies
#include $(COMMON)/crypto/.deps
#include $(COMMON)/network/.deps
include $(COMMON)/utils/.deps
$(OBJDIR)/file_utils_test.o: file_utils.h Makefile $(COMMON)/build/.global_rules

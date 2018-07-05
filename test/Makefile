
TARGET = shp_test
OBJDIR = obj

INC    = -I ../external/googletest/googletest/include
LIB    = -L ../external/googletest/googletest/make -l:gtest.a -l:gtest_main.a -lpthread

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

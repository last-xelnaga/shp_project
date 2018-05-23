PREFIX	=

-include makefile.prefix

TARGET = shp_client2
FLAGS = -O0 -g -ggdb

OBJDIR = $(shell pwd)/obj
INC =
LIB =

ifeq ($(WIRING_LIB) , yes)
    FLAGS += -DUSE_WIRINGPI_LIB
    INC += -I./../external/wiringPi/wiringPi
    LIB += -L./../external/wiringPi/wiringPi/ -lwiringPi -lrt
endif


SRCCPP := \
    main.cpp \


# sources
include ../common/log/.rules
include ../common/network/.rules
include ../common/utils/.rules

# global rules
include ../common/build/.rules

# dependencies
include ../common/log/.debs
include ../common/network/.debs
$(OBJDIR)/main.o: log.h Makefile


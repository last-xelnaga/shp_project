PREFIX	=

-include makefile.prefix

TARGET = shp_client
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
    network_manager_class.cpp \
    settings_class.cpp

SRCC := \
    sensors.c

# sources
include ../common/log/.rules
include ../common/network/.rules
include ../common/utils/.rules

# global rules
include ../common/build/.rules

# dependencies
include ../common/log/.debs
include ../common/network/.debs
$(OBJDIR)/main.o: network_manager_class.hpp log.h sensors.h settings_class.hpp Makefile
$(OBJDIR)/network_manager_class.o: client_socket_class.hpp log.h network_manager_class.hpp settings_class.hpp Makefile
$(OBJDIR)/sensors.o: log.h sensors.h Makefile
$(OBJDIR)/settings_class.o: file.h log.h settings_class.hpp Makefile

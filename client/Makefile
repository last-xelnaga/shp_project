PREFIX	=

-include makefile.prefix

TARGET = shp_client
FLAGS = -O0 -g -ggdb

OBJDIR = $(shell pwd)/obj
INC = -I./../external/wiringPi/wiringPi
LIB = -L./../external/wiringPi/wiringPi/ -lwiringPi -lrt

SRCCPP := \
    main.cpp \
    network_manager_class.cpp \
    settings_class.cpp

SRCC := \
    sensor_dht.c \
    sensor_liquid_level.c \
    sensor_pump.c

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
$(OBJDIR)/sensor_dht.o: log.h sensors.h Makefile
$(OBJDIR)/sensor_liquid_level.o: log.h sensors.h Makefile
$(OBJDIR)/sensor_pump.o: log.h sensors.h Makefile
$(OBJDIR)/settings_class.o: file.h log.h settings_class.hpp Makefile

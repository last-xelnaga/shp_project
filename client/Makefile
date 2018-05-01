PREFIX	=

-include makefile.prefix

#REVFILE = REVISION
TARGET  = shp_client
FLAGS = -O0 -g -ggdb

OBJDIR = $(shell pwd)/obj
INC = -I.
LIB =

SRCCPP := \
    main.cpp \
    network_manager_class.cpp

SRCC := \
    sensor_dht.c \
    sensor_liquid_level.c \
    sensor_pump.c

# sources
include ../common/log/.rules
include ../common/network/.rules

# global rules
include ../common/build/.rules

# dependencies
include ../common/log/.debs
include ../common/network/.debs
$(OBJDIR)/main.o: network_manager_class.hpp log.h sensors.h Makefile
$(OBJDIR)/network_manager_class.o: client_socket_class.hpp log.h network_manager_class.hpp Makefile
$(OBJDIR)/sensor_dht.o: sensors.h Makefile
$(OBJDIR)/sensor_liquid_level.o: sensors.h Makefile
$(OBJDIR)/sensor_pump.o: sensors.h Makefile

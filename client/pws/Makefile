
PREFIX =

-include ../../makefile.prefix_arm

TARGET = shp_client
OBJDIR = $(shell pwd)/obj
INC    =
LIB    =

ifeq ($(RPI_TARGET), yes)
    FLAGS += -DRPI_TARGET
endif

SRCCPP = main.cpp

COMMON = ../../common

# sources
include $(COMMON)/network/.client_rules
include $(COMMON)/sensors/.sensors_rules
include $(COMMON)/utils/.utils_rules

# global rules
include $(COMMON)/build/.global_rules

# dependencies
include $(COMMON)/network/.deps
include $(COMMON)/sensors/.deps
include $(COMMON)/utils/.deps
$(OBJDIR)/main.o: log.h network_manager_class.hpp rpi_gpio.h rpi_spi.h sensor_buzzer.h sensor_dht22.h settings_class.hpp sys_utils.h Makefile

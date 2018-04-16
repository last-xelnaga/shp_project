PREFIX	=

-include makefile.prefix

#REVFILE = REVISION
TARGET  = shp_client
FLAGS = -O0 -g -ggdb

OBJDIR = $(shell pwd)/obj
INC = -I.
LIB = 

SRCCPP := \
    main.cpp

# sources
include ../common/log/.rules
include ../common/network/.rules

# global rules
include ../common/build/.rules

# dependencies
include ../common/log/.debs
include ../common/network/.debs
$(OBJDIR)/main.o: log.h client_socket_class.hpp Makefile

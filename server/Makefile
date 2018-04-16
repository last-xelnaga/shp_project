PREFIX	=
-include makefile.prefix

#REVFILE = REVISION
TARGET = shp_server
FLAGS = -O0 -g -ggdb
#CFLAGS	+= -O2 -fno-omit-frame-pointer

OBJDIR = $(shell pwd)/obj
INC = -I. -I./../external/cpp-json/include
LIB = -lcurl -lssl -lcrypto

SRCCPP := \
    main.cpp \
    fcm_messaging_class.cpp

# sources
include ../common/log/.rules
include ../common/network/.rules
include ../common/utils/.rules

# global rules
include ../common/build/.rules

# dependencies
include ../common/log/.debs
include ../common/network/.debs
include ../common/utils/.debs
$(OBJDIR)/main.o: log.h fcm_messaging_class.hpp server_socket_class.hpp Makefile
$(OBJDIR)/fcm_messaging_class.o: base64.h fcm_messaging_class.hpp file.h log.h sha256.h Makefile

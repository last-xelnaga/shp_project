
TARGET = shp_server
FLAGS  = -O0 -g3 -ggdb
#CFLAGS	+= -O2 -fno-omit-frame-pointer

OBJDIR = $(shell pwd)/obj
INC    = -I.
LIB    =

# add cpp-json support
INC   += -I./../external/cpp-json/include

# add static curl
INC   += -I../external/curl-7.54.0/install/include
LIB   += -L../external/curl-7.54.0/install/lib -l:libcurl.a

# add static openssl
INC   += -I../external/openssl-1.1.0e/install/include
LIB   += -L../external/openssl-1.1.0e/install/lib -l:libssl.a -l:libcrypto.a -ldl

SRCCPP = \
    fcm_messaging_class.cpp \
    main.cpp \
    server_worker.cpp

COMMON = ../common

# sources
include $(COMMON)/crypto/.crypto_rules
include $(COMMON)/network/.server_rules
include $(COMMON)/utils/.utils_rules

# global rules
include $(COMMON)/build/.global_rules

# dependencies
include $(COMMON)/crypto/.deps
include $(COMMON)/network/.deps
include $(COMMON)/utils/.deps
$(OBJDIR)/fcm_messaging_class.o: base64.h fcm_messaging_class.hpp file_utils.h log.h sha256.h Makefile
$(OBJDIR)/main.o: log.h server_socket_class.hpp server_worker.hpp Makefile
$(OBJDIR)/server_worker.o: fcm_messaging_class.hpp log.h server_socket_class.hpp server_worker.hpp Makefile

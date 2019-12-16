
TARGET = shp_server
OBJDIR = $(shell pwd)/obj

-include ../makefile.prefix_arm
#-include ../makefile.prefix_linux


ifeq ($(RPI_TARGET), yes)
    FLAGS += -DRPI_TARGET
	LIB +=  -lrt
endif

#FLAGS  = -DLINUX_TARGET

# add cpp-json support
INC   += -I./../external/cpp-json/lib/include

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
$(OBJDIR)/fcm_messaging_class.o: base64.h fcm_messaging_class.hpp file_utils.h log.h sha256.h Makefile $(COMMON)/build/.global_rules
$(OBJDIR)/main.o: log.h server_socket_class.hpp server_worker.hpp Makefile $(COMMON)/build/.global_rules
$(OBJDIR)/server_worker.o: fcm_messaging_class.hpp log.h server_socket_class.hpp server_worker.hpp Makefile $(COMMON)/build/.global_rules

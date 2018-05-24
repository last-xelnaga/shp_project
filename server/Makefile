
TARGET = shp_server
FLAGS = -O0 -g3 -ggdb
#CFLAGS	+= -O2 -fno-omit-frame-pointer

OBJDIR = $(shell pwd)/obj
INC = -I.
LIB =

# add cpp-json support
INC += -I./../external/cpp-json/include

# add static openssl
INC += -I../external/openssl-1.1.0e/install/include
LIB += -L../external/openssl-1.1.0e/install/lib -lssl -lcrypto -ldl

# add static curl
INC += -I../external/curl-7.54.0/install/include
LIB += -L../external/curl-7.54.0/install/lib -lcurl

SRCCPP := \
    fcm_messaging_class.cpp \
    main.cpp \
    server_worker.cpp

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
$(OBJDIR)/fcm_messaging_class.o: base64.h fcm_messaging_class.hpp file.h log.h sha256.h Makefile
$(OBJDIR)/main.o: log.h server_socket_class.hpp server_worker.hpp Makefile
$(OBJDIR)/server_worker.o: fcm_messaging_class.hpp log.h server_socket_class.hpp server_worker.hpp Makefile

.PHONY: deploy

deploy: all
	strip $(TARGET)
	./deploy_to_aws.sh

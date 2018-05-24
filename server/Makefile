
TARGET = shp_server
FLAGS = -O0 -g -ggdb
#CFLAGS	+= -O2 -fno-omit-frame-pointer

OBJDIR = $(shell pwd)/obj
INC = -I. -I./../external/cpp-json/include
LIB = -lcurl -lssl -lcrypto

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

deploy:
	./deploy_to_aws.sh

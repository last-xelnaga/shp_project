PREFIX	=
-include makefile.prefix

#REVFILE = REVISION
TARGET = shp_server
FLAGS = -O0 -g -ggdb
#CFLAGS	+= -O2 -fno-omit-frame-pointer

INC = -I.
LIB = -lcurl -lssl -lcrypto

SRCCPP := \
    main.cpp \
    fcm_messaging_class.cpp

# sources
include ../common/network/.rules
include ../common/utils/.rules

# global rules
include ../common/build/.rules

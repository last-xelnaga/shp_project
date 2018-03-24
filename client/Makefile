PREFIX	=

-include makefile.prefix

#REVFILE = REVISION
OBJDIR  = ./obj
TARGET  = shp_client

CC	= $(PREFIX)g++

CFLAGS	= -Wall -Wextra -DDEBUG_TAG="shp_client"
CFLAGS	+= -O2 -fno-omit-frame-pointer
#CFLAGS	+= -O0 -ggdb
#CFLAGS	+= -DDEBUG
#CFLAGS	+= -DTRACE -DDATA


INC = -I. -I../common/
#LIB = -lpthread

SRC := \
    main.cpp \
    socket_client.cpp \
    socket.cpp

VPATH = src:../common

OBJ = $(SRC:%.cpp=$(OBJDIR)/%.o)

$(OBJDIR)/%.o :%.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: all clean

all: $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJDIR) $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIB)

clean:
	rm -rf $(OBJDIR) $(TARGET)

# valgrind --leak-check=full --show-leak-kinds=all ./shp_client

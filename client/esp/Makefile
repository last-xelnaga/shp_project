
include ../makefile.prefix_esp

PROJECT_NAME   = esp_test
ROOT_DIR       = $(shell pwd)
BUILD_DIR_BASE = $(ROOT_DIR)/obj

EXTRA_CFLAGS   = \
    -DDEBUG_TAG="$(PROJECT_NAME)" \
    -DESP_TARGET

EXTRA_COMPONENT_DIRS = \
    $(ROOT_DIR) \
    $(ROOT_DIR)/../../common/sensors \
    $(ROOT_DIR)/../../common/utils

include $(IDF_PATH)/make/project.mk

#make clean
#make menuconfig
#make all
#make flash monitor

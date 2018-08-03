
include ../makefile.prefix_esp

PROJECT_NAME := esp_test

.PHONY: conf

conf:
	cp sdkconfig.temp sdkconfig
	sed -i 's\xtensa-esp32-elf-\${PREFIX}\g' sdkconfig


include $(IDF_PATH)/make/project.mk

#make menuconfig
#make all
#make flash

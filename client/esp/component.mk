#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

ifeq ("$(wildcard $(PROJECT_PATH)/wifi_data.h)","")
    $(shell touch $(PROJECT_PATH)/wifi_data.h)
    $(shell echo '#define WIFI_SSID "your_ssid"' > $(PROJECT_PATH)/wifi_data.h)
    $(shell echo '#define WIFI_PASSWORD "your_password"' >> $(PROJECT_PATH)/wifi_data.h)
endif

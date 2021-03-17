PROJECT_NAME = $(shell basename "$(realpath ./)")

# Configurations
NRF_IC = nrf52840
SDK_VERSION = 15
SOFTDEVICE_MODEL = s140

# Source and header files
APP_HEADER_PATHS += .
APP_HEADER_PATHS += ../EWMA_filter/
APP_HEADER_PATHS += ../payload_lib/

APP_SOURCE_PATHS += .
APP_SOURCE_PATHS += ../EWMA_filter/
APP_SOURCE_PATHS += ../payload_lib/

# Custom ECG library
EWMA_SOURCES += ../EWMA_filter/ewma.c
PAYLOAD_SOURCES += ./payload_lib/payload.c

APP_SOURCES = $(notdir $(wildcard ./*.c)) $(notdir $(EWMA_SOURCES)) $(notdir $(PAYLOAD_SOURCES))

# Path to base of nRF52x-base repo
NRF_BASE_DIR = ../../../nrf52x-base/

# Include board Makefile (if any)
include ../../../boards/nrf52840dk-ble/Board.mk

# Include main Makefile
include $(NRF_BASE_DIR)/make/AppMakefile.mk

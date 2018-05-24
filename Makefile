#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := joltwallet

#EXTRA_COMPONENT_DIRS := $(IDF_PATH)/../third-party-components

#COMPONENT_ADD_INCLUDEDIRS += main/menus
#COMPONENT_ADD_SRCDIRS += main/menus

CFLAGS +=  \
	 -I$(BUILD_DIR_BASE)/libwebsockets/include \
	 -I$(PROJECT_PATH)/components/libwebsockets/lib \
	 -D U8G2_16BIT

include $(IDF_PATH)/make/project.mk

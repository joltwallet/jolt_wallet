#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := jolt_wallet

EXTRA_COMPONENT_DIRS := jolt_wallet

#COMPONENTS = 
CFLAGS +=  \
	 -D U8G2_16BIT

include $(IDF_PATH)/make/project.mk

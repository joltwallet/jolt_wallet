#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := jolt_wallet

EXTRA_COMPONENT_DIRS := \
	$(abspath jolt_wallet)

#COMPONENTS = 
#CFLAGS += 

include $(IDF_PATH)/make/project.mk

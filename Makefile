PROJECT_NAME := jolt_os

EXTRA_COMPONENT_DIRS := \
	$(abspath jolt_os)

GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

#COMPONENTS = 
	#
CFLAGS += \
		  -DJOLT_OS \
		  -Werror \
		  -DJOLT_OS_COMMIT_STR=\"$(GIT_VERSION)\" \
		  -DJOLT_GUI_TEST_MENU=false \
		  -DJOLT_GUI_DEBUG_FUNCTIONS=false

include $(IDF_PATH)/make/project.mk

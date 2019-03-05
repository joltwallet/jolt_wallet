PROJECT_NAME := jolt_os

EXTRA_COMPONENT_DIRS := \
	$(abspath jolt_os)

#COMPONENTS = 
CFLAGS += \
		  -Werror \
		  -DJOLT_OS \
		  -DJOLT_GUI_TEST_MENU \
		  -DJOLT_GUI_DEBUG_FUNCTIONS

include $(IDF_PATH)/make/project.mk
